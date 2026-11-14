#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

HaloEngineAudioProcessor::HaloEngineAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    for (int i = 0; i < numVoices; ++i)
    {
        // Warm analog-style sine + subtle 2nd harmonic
        warmOscs[i].initialise ([] (float x) {
            return std::sin (x) + 0.18f * std::sin (2.0f * x);
        }, 128);

        // Glass choir formant-rich harmonic blend
        choirOscs[i].initialise ([] (float x) {
            return 0.55f * std::sin (x)
                 + 0.30f * std::sin (2.0f * x)
                 + 0.15f * std::sin (3.0f * x);
        }, 128);
    }
}

HaloEngineAudioProcessor::~HaloEngineAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout HaloEngineAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    juce::StringArray keys { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("DRONE_KEY", "Drone Key", keys, 0));
    params.push_back (std::make_unique<juce::AudioParameterBool>   ("DRONE_ON", "Drone Active", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("DRONE_LEVEL", "Drone Level", 0.0f, 1.0f, 0.65f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("MORPH_X", "Texture Morph X", 0.0f, 1.0f, 0.35f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("MORPH_Y", "Brightness Morph Y", 0.0f, 1.0f, 0.45f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("SHIMMER_AMT", "Shimmer Octave", 0.0f, 1.0f, 0.50f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("AMBIENCE_DECAY", "Space Decay", 0.0f, 1.0f, 0.85f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("AUTO_DUCK", "Band Auto-Duck", 0.0f, 1.0f, 0.40f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("AMBIENCE_MIX", "Ambience Mix", 0.0f, 1.0f, 0.55f));

    params.push_back (std::make_unique<juce::AudioParameterBool>   ("PRAYER_MACRO", "Prayer / Transition", false));
    params.push_back (std::make_unique<juce::AudioParameterBool>   ("KEY_DETECT_ON", "Smart Key Assist", true));

    return { params.begin(), params.end() };
}

const juce::String HaloEngineAudioProcessor::getName() const { return JucePlugin_Name; }
bool HaloEngineAudioProcessor::acceptsMidi() const { return true; }
bool HaloEngineAudioProcessor::producesMidi() const { return false; }
bool HaloEngineAudioProcessor::isMidiEffect() const { return false; }
double HaloEngineAudioProcessor::getTailLengthSeconds() const { return 4.0; }

int HaloEngineAudioProcessor::getNumPrograms() { return 1; }
int HaloEngineAudioProcessor::getCurrentProgram() { return 0; }
void HaloEngineAudioProcessor::setCurrentProgram (int) {}
const juce::String HaloEngineAudioProcessor::getProgramName (int) { return {}; }
void HaloEngineAudioProcessor::changeProgramName (int, const juce::String&) {}

void HaloEngineAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = 1;

    for (int i = 0; i < numVoices; ++i)
    {
        warmOscs[i].prepare (spec);
        choirOscs[i].prepare (spec);
        voiceFreqSmoothed[i].reset (sampleRate, 0.80); // 800ms smooth musical glide
    }

    droneFilterL.prepare (spec);
    droneFilterR.prepare (spec);
    droneFilterL.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    droneFilterR.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    shimmerFilterL.prepare (spec);
    shimmerFilterR.prepare (spec);
    shimmerFilterL.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
    shimmerFilterR.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
    shimmerFilterL.setCutoffFrequency (2400.0f);
    shimmerFilterR.setCutoffFrequency (2400.0f);

    // 100ms granular shimmer buffer
    int pitchBufSize = static_cast<int> (sampleRate * 0.10);
    pitchBufferL.assign (static_cast<size_t> (pitchBufSize), 0.0f);
    pitchBufferR.assign (static_cast<size_t> (pitchBufSize), 0.0f);
    pitchWriteIdx = 0;
    pitchReadPhase = 0.0f;

    stereoReverb.setSampleRate (sampleRate);
    stereoReverb.reset();

    droneGainSmoothed.reset (sampleRate, 0.15);
    prayerMacroSmoothed.reset (sampleRate, 2.50); // 2.5s silky prayer swell curve
    morphXSmoothed.reset (sampleRate, 0.08);
    morphYSmoothed.reset (sampleRate, 0.08);

    detectBuffer.assign (2048, 0.0f);
    detectWritePos = 0;

    int initialKey = static_cast<int> (*apvts.getRawParameterValue ("DRONE_KEY"));
    updateDroneFrequencies (initialKey, true);
    lastKeyIndex = initialKey;
}

void HaloEngineAudioProcessor::releaseResources() {}

bool HaloEngineAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void HaloEngineAudioProcessor::updateDroneFrequencies (int keyIndex, bool snapImmediately)
{
    // MIDI note for C2 = 36
    double rootMidi = 36.0 + static_cast<double> (keyIndex);
    float rootHz = static_cast<float> (440.0 * std::pow (2.0, (rootMidi - 69.0) / 12.0));

    // Worship harmonic voicing: Sub (0.5x), Root (1x), 5th (1.5x), Oct (2x), 9th (2.25x), High 5th (3x)
    std::array<float, numVoices> targetFreqs {
        rootHz * 0.5f,
        rootHz * 1.0f,
        rootHz * 1.4983f, // Just-intonation-friendly fifth
        rootHz * 2.0f,
        rootHz * 2.2449f, // Lush suspended 9th
        rootHz * 2.9966f
    };

    for (int i = 0; i < numVoices; ++i)
    {
        if (snapImmediately)
            voiceFreqSmoothed[i].setCurrentAndTargetValue (targetFreqs[i]);
        else
            voiceFreqSmoothed[i].setTargetValue (targetFreqs[i]);
    }
}

void HaloEngineAudioProcessor::analyzeIncomingPitch()
{
    const int N = static_cast<int> (detectBuffer.size());
    float energy = 0.0f;
    for (float s : detectBuffer)
        energy += s * s;

    // Ignore quiet room noise
    if (energy / static_cast<float> (N) < 0.0004f)
        return;

    int minLag = static_cast<int> (currentSampleRate / 600.0); // 600 Hz max
    int maxLag = static_cast<int> (currentSampleRate / 75.0);  // 75 Hz min
    maxLag = juce::jmin (maxLag, N / 2);

    float bestCorr = 0.0f;
    int bestLag = -1;

    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        float corr = 0.0f;
        for (int i = 0; i < N - maxLag; ++i)
            corr += detectBuffer[static_cast<size_t> (i)] * detectBuffer[static_cast<size_t> (i + lag)];

        if (corr > bestCorr)
        {
            bestCorr = corr;
            bestLag = lag;
        }
    }

    if (bestLag > 0)
    {
        double detectedHz = currentSampleRate / static_cast<double> (bestLag);
        double midiNote = 69.0 + 12.0 * std::log2 (detectedHz / 440.0);
        int noteClass = (static_cast<int> (std::round (midiNote)) % 12 + 12) % 12;

        if (noteClass == candidateKey)
        {
            candidateConfirmCount++;
            // Require ~1.2 seconds of consistent playing before suggesting a modulation
            if (candidateConfirmCount >= 24)
                suggestedKeyIndex.store (noteClass, std::memory_order_relaxed);
        }
        else
        {
            candidateKey = noteClass;
            candidateConfirmCount = 1;
        }
    }
}

void HaloEngineAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    // Read atomic parameters
    int   keyIndex    = static_cast<int> (*apvts.getRawParameterValue ("DRONE_KEY"));
    bool  droneActive = *apvts.getRawParameterValue ("DRONE_ON") > 0.5f;
    float droneLevel  = *apvts.getRawParameterValue ("DRONE_LEVEL");
    float morphX      = *apvts.getRawParameterValue ("MORPH_X");
    float morphY      = *apvts.getRawParameterValue ("MORPH_Y");
    float shimmerAmt  = *apvts.getRawParameterValue ("SHIMMER_AMT");
    float decayAmt    = *apvts.getRawParameterValue ("AMBIENCE_DECAY");
    float autoDuck    = *apvts.getRawParameterValue ("AUTO_DUCK");
    float ambMix      = *apvts.getRawParameterValue ("AMBIENCE_MIX");
    bool  prayerOn    = *apvts.getRawParameterValue ("PRAYER_MACRO") > 0.5f;
    bool  detectOn    = *apvts.getRawParameterValue ("KEY_DETECT_ON") > 0.5f;

    if (keyIndex != lastKeyIndex)
    {
        updateDroneFrequencies (keyIndex, false);
        lastKeyIndex = keyIndex;
    }

    droneGainSmoothed.setTargetValue (droneActive ? droneLevel : 0.0f);
    prayerMacroSmoothed.setTargetValue (prayerOn ? 1.0f : 0.0f);
    morphXSmoothed.setTargetValue (morphX);
    morphYSmoothed.setTargetValue (morphY);

    // Configure lush stereo reverb
    reverbParams.roomSize   = juce::jlimit (0.55f, 0.98f, 0.65f + decayAmt * 0.32f + (prayerOn ? 0.06f : 0.0f));
    reverbParams.damping    = prayerOn ? 0.65f : (0.55f - morphY * 0.35f);
    reverbParams.wetLevel   = 1.0f;
    reverbParams.dryLevel   = 0.0f;
    reverbParams.width      = 1.0f;
    reverbParams.freezeMode = 0.0f;
    stereoReverb.setParameters (reverbParams);

    juce::AudioBuffer<float> wetBuffer (2, numSamples);
    wetBuffer.clear();

    auto* inL  = totalNumInputChannels > 0 ? buffer.getReadPointer (0) : nullptr;
    auto* inR  = totalNumInputChannels > 1 ? buffer.getReadPointer (1) : inL;
    auto* outL = buffer.getWritePointer (0);
    auto* outR = totalNumOutputChannels > 1 ? buffer.getWritePointer (1) : outL;
    auto* wetL = wetBuffer.getWritePointer (0);
    auto* wetR = wetBuffer.getWritePointer (1);

    const float envAttack  = std::exp (-1.0f / (static_cast<float> (currentSampleRate) * 0.015f));
    const float envRelease = std::exp (-1.0f / (static_cast<float> (currentSampleRate) * 0.350f));
    const int pitchBufSize = static_cast<int> (pitchBufferL.size());

    // Voice level weights (Sub, Root, 5th, Oct, 9th, High 5th)
    const std::array<float, numVoices> voiceWeights { 0.30f, 0.32f, 0.20f, 0.16f, 0.12f, 0.08f };

    for (int s = 0; s < numSamples; ++s)
    {
        float dryL = inL != nullptr ? inL[s] : 0.0f;
        float dryR = inR != nullptr ? inR[s] : 0.0f;
        float monoIn = 0.5f * (dryL + dryR);

        // Feed non-intrusive pitch detector
        if (detectOn)
        {
            detectBuffer[static_cast<size_t> (detectWritePos++)] = monoIn;
            if (detectWritePos >= static_cast<int> (detectBuffer.size()))
            {
                analyzeIncomingPitch();
                detectWritePos = 0;
            }
        }
        else
        {
            suggestedKeyIndex.store (-1, std::memory_order_relaxed);
        }

        // Track live band energy for reactive auto-ducking
        float absIn = std::abs (monoIn);
        if (absIn > inputEnvelope)
            inputEnvelope = envAttack * inputEnvelope + (1.0f - envAttack) * absIn;
        else
            inputEnvelope = envRelease * inputEnvelope + (1.0f - envRelease) * absIn;

        float prayerVal = prayerMacroSmoothed.getNextValue();
        float curX      = morphXSmoothed.getNextValue();
        float curY      = juce::jlimit (0.0f, 1.0f, morphYSmoothed.getNextValue() * (1.0f - 0.55f * prayerVal));
        float dGain     = droneGainSmoothed.getNextValue() * (1.0f + 0.25f * prayerVal);

        // Synthesize 6-voice harmonic drone with stereo spread
        float droneSumL = 0.0f;
        float droneSumR = 0.0f;

        for (int v = 0; v < numVoices; ++v)
        {
            float f = voiceFreqSmoothed[v].getNextValue();
            warmOscs[v].setFrequency (f, false);
            choirOscs[v].setFrequency (f, false);

            float wSample = warmOscs[v].processSample (0.0f);
            float cSample = choirOscs[v].processSample (0.0f);
            float morphed = (1.0f - curX) * wSample + curX * cSample;

            // Boost sub-octave slightly during Prayer/Transition mode
            float weight = voiceWeights[v] * (v == 0 ? (1.0f + 0.5f * prayerVal) : 1.0f);
            float pan = (v % 2 == 0) ? 0.42f : 0.58f;

            droneSumL += morphed * weight * (1.0f - pan);
            droneSumR += morphed * weight * pan;
        }

        // Dynamic warmth filter controlled by Morph Y and Prayer Macro
        float cutoffHz = 260.0f + std::pow (curY, 1.8f) * 4200.0f;
        droneFilterL.setCutoffFrequency (cutoffHz);
        droneFilterR.setCutoffFrequency (cutoffHz);

        float filteredDroneL = droneFilterL.processSample (0, droneSumL) * dGain * 0.45f;
        float filteredDroneR = droneFilterR.processSample (0, droneSumR) * dGain * 0.45f;

        // Combine incoming instrument + drone into the Shimmer Ambience tank
        float tankInL = dryL + filteredDroneL * 0.65f;
        float tankInR = dryR + filteredDroneR * 0.65f;

        // Dual-Head Granular Octave-Up Pitch Shifter for Shimmer
        pitchBufferL[static_cast<size_t> (pitchWriteIdx)] = tankInL;
        pitchBufferR[static_cast<size_t> (pitchWriteIdx)] = tankInR;

        float phaseA = pitchReadPhase;
        float phaseB = std::fmod (pitchReadPhase + 0.5f, 1.0f);

        float offsetA = (1.0f - phaseA) * static_cast<float> (pitchBufSize - 2);
        float offsetB = (1.0f - phaseB) * static_cast<float> (pitchBufSize - 2);

        int idxA = (pitchWriteIdx - static_cast<int> (offsetA) + pitchBufSize) % pitchBufSize;
        int idxB = (pitchWriteIdx - static_cast<int> (offsetB) + pitchBufSize) % pitchBufSize;

        float winA = std::sin (phaseA * juce::MathConstants<float>::pi);
        float winB = std::sin (phaseB * juce::MathConstants<float>::pi);

        float octL = pitchBufferL[static_cast<size_t> (idxA)] * winA + pitchBufferL[static_cast<size_t> (idxB)] * winB;
        float octR = pitchBufferR[static_cast<size_t> (idxA)] * winA + pitchBufferR[static_cast<size_t> (idxB)] * winB;

        octL = shimmerFilterL.processSample (0, octL);
        octR = shimmerFilterR.processSample (0, octR);

        pitchWriteIdx = (pitchWriteIdx + 1) % pitchBufSize;
        pitchReadPhase += 1.0f / static_cast<float> (pitchBufSize); // +1 octave read speed
        if (pitchReadPhase >= 1.0f)
            pitchReadPhase -= 1.0f;

        float effectiveShimmer = juce::jlimit (0.0f, 1.0f, shimmerAmt + 0.22f * prayerVal);
        wetL[s] = tankInL + octL * effectiveShimmer * 0.85f;
        wetR[s] = tankInR + octR * effectiveShimmer * 0.85f;

        // Store dry + direct drone temporarily in output buffer
        outL[s] = dryL + filteredDroneL;
        outR[s] = dryR + filteredDroneR;
    }

    // Run lush stereo reverb across the entire shimmer + drone tank
    stereoReverb.processStereo (wetL, wetR, numSamples);

    // Reactive Auto-Ducking: gently tucks wet ambience when the band hits hard, blooms in silence
    float duckGain = 1.0f - juce::jlimit (0.0f, 0.72f, inputEnvelope * autoDuck * 4.5f);
    float finalWetMix = juce::jlimit (0.0f, 1.0f, ambMix + 0.25f * prayerMacroSmoothed.getCurrentValue()) * duckGain;

    for (int s = 0; s < numSamples; ++s)
    {
        outL[s] += wetL[s] * finalWetMix;
        outR[s] += wetR[s] * finalWetMix;
    }

    // Update lock-free UI telemetry meters
    outLevelL.store (buffer.getRMSLevel (0, 0, numSamples), std::memory_order_relaxed);
    outLevelR.store (totalNumOutputChannels > 1 ? buffer.getRMSLevel (1, 0, numSamples)
                                                : outLevelL.load (std::memory_order_relaxed), std::memory_order_relaxed);
    duckMeter.store (juce::jlimit (0.0f, 1.0f, inputEnvelope * autoDuck * 3.5f), std::memory_order_relaxed);
}

bool HaloEngineAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* HaloEngineAudioProcessor::createEditor()
{
    return new PluginEditor (*this);
}

void HaloEngineAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void HaloEngineAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HaloEngineAudioProcessor();
}