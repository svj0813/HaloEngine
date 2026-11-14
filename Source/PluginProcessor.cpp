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
        warmOscs[i].initialise ([] (float x) {
            return std::sin (x) + 0.18f * std::sin (2.0f * x);
        }, 128);

        choirOscs[i].initialise ([] (float x) {
            return 0.55f * std::sin (x)
                 + 0.30f * std::sin (2.0f * x)
                 + 0.15f * std::sin (3.0f * x);
        }, 128);

        detuneOscs[i].initialise ([] (float x) {
            float saw = x / juce::MathConstants<float>::pi;
            return 0.65f * std::sin (x) + 0.35f * saw;
        }, 128);
    }
}

HaloEngineAudioProcessor::~HaloEngineAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout HaloEngineAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Main Performance Parameters
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

    // Synth & Voicing Engine Parameters (Tab 2)
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_SUB",   "Sub Octave",  0.0f, 1.0f, 0.75f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_ROOT",  "Root Voice",  0.0f, 1.0f, 0.85f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_FIFTH", "Fifth Voice", 0.0f, 1.0f, 0.55f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_OCT",   "Octave Voice",0.0f, 1.0f, 0.45f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_NINTH", "Ninth Voice", 0.0f, 1.0f, 0.35f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_HIGH",  "High Fifth",  0.0f, 1.0f, 0.25f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_DETUNE", "Analog Drift",  0.0f, 1.0f, 0.25f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_WIDTH",  "Stereo Width",  0.0f, 1.0f, 0.80f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_RES",    "Filter Res",    0.0f, 1.0f, 0.20f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_DRIVE",  "Warm Drive",    0.0f, 1.0f, 0.15f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_GLIDE",  "Key Glide",     0.05f, 2.5f, 0.80f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_ATTACK", "MIDI Attack",   0.01f, 2.0f, 0.25f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_RELEASE","MIDI Release",  0.05f, 4.0f, 1.20f));

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

    float glideSec = *apvts.getRawParameterValue ("SYNTH_GLIDE");
    lastGlideSec = glideSec;

    for (int i = 0; i < numVoices; ++i)
    {
        warmOscs[i].prepare (spec);
        choirOscs[i].prepare (spec);
        detuneOscs[i].prepare (spec);
        voiceFreqSmoothed[i].reset (sampleRate, glideSec);
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

    int pitchBufSize = static_cast<int> (sampleRate * 0.10);
    pitchBufferL.assign (static_cast<size_t> (pitchBufSize), 0.0f);
    pitchBufferR.assign (static_cast<size_t> (pitchBufSize), 0.0f);
    pitchWriteIdx = 0;
    pitchReadPhase = 0.0f;

    stereoReverb.setSampleRate (sampleRate);
    stereoReverb.reset();

    droneGainSmoothed.reset (sampleRate, 0.15);
    prayerMacroSmoothed.reset (sampleRate, 2.50);
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
    double rootMidi = 36.0 + static_cast<double> (keyIndex);
    float rootHz = static_cast<float> (440.0 * std::pow (2.0, (rootMidi - 69.0) / 12.0));

    std::array<float, numVoices> targetFreqs {
        rootHz * 0.5f,
        rootHz * 1.0f,
        rootHz * 1.4983f,
        rootHz * 2.0f,
        rootHz * 2.2449f,
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

    if (energy / static_cast<float> (N) < 0.0004f)
        return;

    int minLag = static_cast<int> (currentSampleRate / 600.0);
    int maxLag = static_cast<int> (currentSampleRate / 75.0);
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

void HaloEngineAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    // Inject mouse clicks from Synth Tab keyboard into incoming MIDI stream
    keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            int note = msg.getNoteNumber();
            int slot = 0;
            for (int v = 0; v < maxMidiVoices; ++v)
            {
                if (! midiVoices[static_cast<size_t> (v)].active)
                {
                    slot = v;
                    break;
                }
            }
            auto& mv = midiVoices[static_cast<size_t> (slot)];
            mv.active = true;
            mv.releasing = false;
            mv.midiNote = note;
            mv.freq = static_cast<float> (440.0 * std::pow (2.0, (note - 69.0) / 12.0));
        }
        else if (msg.isNoteOff())
        {
            int note = msg.getNoteNumber();
            for (auto& mv : midiVoices)
            {
                if (mv.active && mv.midiNote == note)
                    mv.releasing = true;
            }
        }
        else if (msg.isAllNotesOff())
        {
            for (auto& mv : midiVoices)
                mv.releasing = true;
        }
    }

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

    std::array<float, numVoices> voiceWeights {
        *apvts.getRawParameterValue ("VOICE_SUB")   * 0.42f,
        *apvts.getRawParameterValue ("VOICE_ROOT")  * 0.40f,
        *apvts.getRawParameterValue ("VOICE_FIFTH") * 0.34f,
        *apvts.getRawParameterValue ("VOICE_OCT")   * 0.32f,
        *apvts.getRawParameterValue ("VOICE_NINTH") * 0.28f,
        *apvts.getRawParameterValue ("VOICE_HIGH")  * 0.24f
    };

    float detuneAmt   = *apvts.getRawParameterValue ("SYNTH_DETUNE");
    float stereoWidth = *apvts.getRawParameterValue ("SYNTH_WIDTH");
    float filterRes   = *apvts.getRawParameterValue ("SYNTH_RES");
    float warmDrive   = *apvts.getRawParameterValue ("SYNTH_DRIVE");
    float glideSec    = *apvts.getRawParameterValue ("SYNTH_GLIDE");
    float attackSec   = *apvts.getRawParameterValue ("SYNTH_ATTACK");
    float releaseSec  = *apvts.getRawParameterValue ("SYNTH_RELEASE");

    if (std::abs (glideSec - lastGlideSec) > 0.01f)
    {
        for (int i = 0; i < numVoices; ++i)
            voiceFreqSmoothed[i].reset (currentSampleRate, glideSec);
        lastGlideSec = glideSec;
    }

    if (keyIndex != lastKeyIndex)
    {
        updateDroneFrequencies (keyIndex, false);
        lastKeyIndex = keyIndex;
    }

    float resonanceVal = 0.707f + filterRes * 3.8f;
    droneFilterL.setResonance (resonanceVal);
    droneFilterR.setResonance (resonanceVal);

    droneGainSmoothed.setTargetValue (droneActive ? droneLevel : 0.0f);
    prayerMacroSmoothed.setTargetValue (prayerOn ? 1.0f : 0.0f);
    morphXSmoothed.setTargetValue (morphX);
    morphYSmoothed.setTargetValue (morphY);

    reverbParams.roomSize   = juce::jlimit (0.55f, 0.98f, 0.65f + decayAmt * 0.32f + (prayerOn ? 0.06f : 0.0f));
    reverbParams.damping    = prayerOn ? 0.65f : (0.55f - morphY * 0.35f);
    reverbParams.wetLevel   = 1.0f;
    reverbParams.dryLevel   = 0.0f;
    reverbParams.width      = stereoWidth;
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
    const float midiAttRate = 1.0f / (static_cast<float> (currentSampleRate) * juce::jmax (0.005f, attackSec));
    const float midiRelRate = 1.0f / (static_cast<float> (currentSampleRate) * juce::jmax (0.02f, releaseSec));
    const int pitchBufSize = static_cast<int> (pitchBufferL.size());

    for (int s = 0; s < numSamples; ++s)
    {
        float dryL = inL != nullptr ? inL[s] : 0.0f;
        float dryR = inR != nullptr ? inR[s] : 0.0f;
        float monoIn = 0.5f * (dryL + dryR);

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

        float absIn = std::abs (monoIn);
        if (absIn > inputEnvelope)
            inputEnvelope = envAttack * inputEnvelope + (1.0f - envAttack) * absIn;
        else
            inputEnvelope = envRelease * inputEnvelope + (1.0f - envRelease) * absIn;

        float prayerVal = prayerMacroSmoothed.getNextValue();
        float curX      = morphXSmoothed.getNextValue();
        float curY      = juce::jlimit (0.0f, 1.0f, morphYSmoothed.getNextValue() * (1.0f - 0.55f * prayerVal));
        float dGain     = droneGainSmoothed.getNextValue() * (1.0f + 0.25f * prayerVal);

        float droneSumL = 0.0f;
        float droneSumR = 0.0f;

        for (int v = 0; v < numVoices; ++v)
        {
            float f = voiceFreqSmoothed[v].getNextValue();
            float detuneRatio = 1.0f + ((v % 2 == 0 ? 1.0f : -1.0f) * detuneAmt * 0.0065f);

            warmOscs[v].setFrequency (f, false);
            choirOscs[v].setFrequency (f, false);
            detuneOscs[v].setFrequency (f * detuneRatio, false);

            float wSample = warmOscs[v].processSample (0.0f);
            float cSample = choirOscs[v].processSample (0.0f);
            float dSample = detuneOscs[v].processSample (0.0f);

            float morphed = ((1.0f - curX) * wSample + curX * cSample) * (1.0f - detuneAmt * 0.35f)
                          + dSample * (detuneAmt * 0.45f);

            float weight = voiceWeights[static_cast<size_t> (v)] * (v == 0 ? (1.0f + 0.5f * prayerVal) : 1.0f);
            float spread = 0.5f + ((v % 2 == 0) ? -0.45f : 0.45f) * stereoWidth;

            droneSumL += morphed * weight * (1.0f - spread);
            droneSumR += morphed * weight * spread;
        }

        float midiSumL = 0.0f;
        float midiSumR = 0.0f;
        for (int m = 0; m < maxMidiVoices; ++m)
        {
            auto& mv = midiVoices[static_cast<size_t> (m)];
            if (! mv.active)
                continue;

            if (! mv.releasing)
                mv.env = juce::jmin (1.0f, mv.env + midiAttRate);
            else
            {
                mv.env -= midiRelRate;
                if (mv.env <= 0.0f)
                {
                    mv.env = 0.0f;
                    mv.active = false;
                    continue;
                }
            }

            double inc1 = (mv.freq * juce::MathConstants<double>::twoPi) / currentSampleRate;
            double inc2 = (mv.freq * (1.0 + detuneAmt * 0.008) * juce::MathConstants<double>::twoPi) / currentSampleRate;
            mv.phase1 = std::fmod (mv.phase1 + inc1, juce::MathConstants<double>::twoPi);
            mv.phase2 = std::fmod (mv.phase2 + inc2, juce::MathConstants<double>::twoPi);

            float oscA = static_cast<float> (std::sin (mv.phase1) + 0.25 * std::sin (2.0 * mv.phase1));
            float oscB = static_cast<float> (std::sin (mv.phase2) + 0.30 * std::sin (3.0 * mv.phase2));
            float sig  = ((1.0f - curX) * oscA + curX * oscB) * mv.env * 0.25f;

            float pan = 0.5f + ((m % 2 == 0) ? -0.35f : 0.35f) * stereoWidth;
            midiSumL += sig * (1.0f - pan);
            midiSumR += sig * pan;
        }

        droneSumL += midiSumL;
        droneSumR += midiSumR;

        if (warmDrive > 0.01f)
        {
            float driveGain = 1.0f + warmDrive * 3.5f;
            droneSumL = std::tanh (droneSumL * driveGain) / std::tanh (driveGain);
            droneSumR = std::tanh (droneSumR * driveGain) / std::tanh (driveGain);
        }

        float cutoffHz = 260.0f + std::pow (curY, 1.8f) * 4800.0f;
        droneFilterL.setCutoffFrequency (cutoffHz);
        droneFilterR.setCutoffFrequency (cutoffHz);

        float filteredDroneL = droneFilterL.processSample (0, droneSumL) * (dGain + 0.35f) * 0.45f;
        float filteredDroneR = droneFilterR.processSample (0, droneSumR) * (dGain + 0.35f) * 0.45f;

        if (s % 4 == 0)
        {
            scopeSamples[static_cast<size_t> (scopeWriteIdx)].store (
                juce::jlimit (-1.0f, 1.0f, (filteredDroneL + filteredDroneR) * 1.8f),
                std::memory_order_relaxed);
            scopeWriteIdx = (scopeWriteIdx + 1) % scopeSize;
        }

        float tankInL = dryL + filteredDroneL * 0.65f;
        float tankInR = dryR + filteredDroneR * 0.65f;

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
        pitchReadPhase += 1.0f / static_cast<float> (pitchBufSize);
        if (pitchReadPhase >= 1.0f)
            pitchReadPhase -= 1.0f;

        float effectiveShimmer = juce::jlimit (0.0f, 1.0f, shimmerAmt + 0.22f * prayerVal);
        wetL[s] = tankInL + octL * effectiveShimmer * 0.85f;
        wetR[s] = tankInR + octR * effectiveShimmer * 0.85f;

        outL[s] = dryL + filteredDroneL;
        outR[s] = dryR + filteredDroneR;
    }

    stereoReverb.processStereo (wetL, wetR, numSamples);

    float duckGain = 1.0f - juce::jlimit (0.0f, 0.72f, inputEnvelope * autoDuck * 4.5f);
    float finalWetMix = juce::jlimit (0.0f, 1.0f, ambMix + 0.25f * prayerMacroSmoothed.getCurrentValue()) * duckGain;

    for (int s = 0; s < numSamples; ++s)
    {
        outL[s] += wetL[s] * finalWetMix;
        outR[s] += wetR[s] * finalWetMix;
    }

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