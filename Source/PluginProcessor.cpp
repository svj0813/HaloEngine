#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <random>

HaloEngineAudioProcessor::HaloEngineAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    std::mt19937 rng (42);
    std::uniform_real_distribution<double> dist (0.0, 1.0);

    for (int v = 0; v < numVoices; ++v)
    {
        droneSubPhases[static_cast<size_t> (v)] = dist (rng);
        for (int u = 0; u < unisonCount; ++u)
            dronePhases[static_cast<size_t> (v)][static_cast<size_t> (u)] = dist (rng);
    }

    for (auto& mv : midiVoices)
    {
        mv.subPhase = dist (rng);
        for (int u = 0; u < unisonCount; ++u)
            mv.phases[static_cast<size_t> (u)] = dist (rng);
    }
}

HaloEngineAudioProcessor::~HaloEngineAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout HaloEngineAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    juce::StringArray keys { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("DRONE_KEY", "Drone Key", keys, 0));
    params.push_back (std::make_unique<juce::AudioParameterBool>   ("DRONE_ON", "Drone Active", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("DRONE_LEVEL", "Drone Level", 0.0f, 1.0f, 0.72f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("MORPH_X", "Texture Morph X", 0.0f, 1.0f, 0.42f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("MORPH_Y", "Brightness Morph Y", 0.0f, 1.0f, 0.48f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("SHIMMER_AMT", "Shimmer Octave", 0.0f, 1.0f, 0.48f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("AMBIENCE_DECAY", "Space Decay", 0.0f, 1.0f, 0.86f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("AUTO_DUCK", "Band Auto-Duck", 0.0f, 1.0f, 0.40f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("AMBIENCE_MIX", "Ambience Mix", 0.0f, 1.0f, 0.58f));

    params.push_back (std::make_unique<juce::AudioParameterBool>   ("PRAYER_MACRO", "Prayer / Transition", false));
    params.push_back (std::make_unique<juce::AudioParameterBool>   ("KEY_DETECT_ON", "Smart Key Assist", true));

    // Supermassive FDN Space Parameters
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("SPACE_WARP", "Cloud Warp", 0.0f, 1.0f, 0.72f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("SPACE_MOD",  "Tank Mod",   0.0f, 1.0f, 0.55f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("SPACE_DAMP", "Air Damp",   0.0f, 1.0f, 0.42f));

    // Synth & Voicing Engine Parameters
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_SUB",   "Sub Octave",  0.0f, 1.0f, 0.80f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_ROOT",  "Root Voice",  0.0f, 1.0f, 0.85f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_FIFTH", "Fifth Voice", 0.0f, 1.0f, 0.52f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_OCT",   "Octave Voice",0.0f, 1.0f, 0.48f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_NINTH", "Ninth Voice", 0.0f, 1.0f, 0.34f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_HIGH",  "High Fifth",  0.0f, 1.0f, 0.22f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_DETUNE", "Unison Detune", 0.0f, 1.0f, 0.38f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_WIDTH",  "Stereo Width",  0.0f, 1.0f, 0.90f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_RES",    "Filter Res",    0.0f, 1.0f, 0.22f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_DRIVE",  "Analog Drive",  0.0f, 1.0f, 0.25f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_GLIDE",  "Key Glide",     0.05f, 2.5f, 0.85f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_ATTACK", "MIDI Attack",   0.01f, 2.0f, 0.32f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_RELEASE","MIDI Release",  0.05f, 4.0f, 1.45f));

    return { params.begin(), params.end() };
}

const juce::String HaloEngineAudioProcessor::getName() const { return JucePlugin_Name; }
bool HaloEngineAudioProcessor::acceptsMidi() const { return true; }
bool HaloEngineAudioProcessor::producesMidi() const { return false; }
bool HaloEngineAudioProcessor::isMidiEffect() const { return false; }
double HaloEngineAudioProcessor::getTailLengthSeconds() const { return 6.0; }

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
        voiceFreqSmoothed[i].reset (sampleRate, glideSec);

    for (auto* f : { &droneFilterL1, &droneFilterL2, &droneFilterR1, &droneFilterR2 })
    {
        f->prepare (spec);
        f->setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    }

    dcBlockL.prepare (spec);
    dcBlockR.prepare (spec);
    dcBlockL.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    dcBlockR.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    dcBlockL.setCutoffFrequency (32.0f);
    dcBlockR.setCutoffFrequency (32.0f);

    int chorusSize = static_cast<int> (sampleRate * 0.05) + 16;
    chorusBufL.assign (static_cast<size_t> (chorusSize), 0.0f);
    chorusBufR.assign (static_cast<size_t> (chorusSize), 0.0f);
    chorusWriteIdx = 0;

    // Prepare Phase 1 Supermassive 8-Channel Modulated FDN Tank
    supermassiveTank.prepare (sampleRate);

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

    keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            int note = msg.getNoteNumber();
            int slot = 0;
            float lowestEnv = 10.0f;
            for (int v = 0; v < maxMidiVoices; ++v)
            {
                if (! midiVoices[static_cast<size_t> (v)].active)
                {
                    slot = v;
                    break;
                }
                if (midiVoices[static_cast<size_t> (v)].env < lowestEnv)
                {
                    lowestEnv = midiVoices[static_cast<size_t> (v)].env;
                    slot = v;
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

    float spaceWarp   = *apvts.getRawParameterValue ("SPACE_WARP");
    float spaceMod    = *apvts.getRawParameterValue ("SPACE_MOD");
    float spaceDamp   = *apvts.getRawParameterValue ("SPACE_DAMP");

    std::array<float, numVoices> voiceWeights {
        *apvts.getRawParameterValue ("VOICE_SUB")   * 0.52f,
        *apvts.getRawParameterValue ("VOICE_ROOT")  * 0.48f,
        *apvts.getRawParameterValue ("VOICE_FIFTH") * 0.36f,
        *apvts.getRawParameterValue ("VOICE_OCT")   * 0.34f,
        *apvts.getRawParameterValue ("VOICE_NINTH") * 0.28f,
        *apvts.getRawParameterValue ("VOICE_HIGH")  * 0.22f
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

    float stageRes = 0.65f + filterRes * 1.95f;
    droneFilterL1.setResonance (stageRes);
    droneFilterL2.setResonance (stageRes * 0.85f);
    droneFilterR1.setResonance (stageRes);
    droneFilterR2.setResonance (stageRes * 0.85f);

    droneGainSmoothed.setTargetValue (droneActive ? droneLevel : 0.0f);
    prayerMacroSmoothed.setTargetValue (prayerOn ? 1.0f : 0.0f);
    morphXSmoothed.setTargetValue (morphX);
    morphYSmoothed.setTargetValue (morphY);

    auto* inL  = totalNumInputChannels > 0 ? buffer.getReadPointer (0) : nullptr;
    auto* inR  = totalNumInputChannels > 1 ? buffer.getReadPointer (1) : inL;
    auto* outL = buffer.getWritePointer (0);
    auto* outR = totalNumOutputChannels > 1 ? buffer.getWritePointer (1) : outL;

    const float envAttack  = std::exp (-1.0f / (static_cast<float> (currentSampleRate) * 0.015f));
    const float envRelease = std::exp (-1.0f / (static_cast<float> (currentSampleRate) * 0.350f));
    const float midiAttRate = 1.0f / (static_cast<float> (currentSampleRate) * juce::jmax (0.005f, attackSec));
    const float midiRelRate = 1.0f / (static_cast<float> (currentSampleRate) * juce::jmax (0.02f, releaseSec));
    const int chorusBufSize = static_cast<int> (chorusBufL.size());
    const double invSr      = 1.0 / currentSampleRate;

    static constexpr std::array<float, unisonCount> unisonOffsets { -1.0f, -0.58f, -0.22f, 0.0f, 0.22f, 0.58f, 1.0f };
    static constexpr std::array<float, unisonCount> unisonPans    { -0.95f, 0.78f, -0.45f, 0.0f, 0.45f, -0.78f, 0.95f };
    static constexpr std::array<float, unisonCount> unisonGains   {  0.22f, 0.28f,  0.34f, 0.42f, 0.34f, 0.28f, 0.22f };

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

        float maxDetuneSemis = (0.04f + detuneAmt * 0.34f) / 12.0f;

        float synthL = 0.0f;
        float synthR = 0.0f;

        if (dGain > 0.0005f)
        {
            for (int v = 0; v < numVoices; ++v)
            {
                float baseFreq = voiceFreqSmoothed[v].getNextValue();
                float weight = voiceWeights[static_cast<size_t> (v)] * (v == 0 ? (1.0f + 0.45f * prayerVal) : 1.0f);
                if (weight < 0.001f)
                    continue;

                double subDt = baseFreq * invSr;
                double subP  = droneSubPhases[static_cast<size_t> (v)] + subDt;
                if (subP >= 1.0) subP -= 1.0;
                droneSubPhases[static_cast<size_t> (v)] = subP;
                float subSine = static_cast<float> (std::sin (subP * juce::MathConstants<double>::twoPi));

                float voiceL = subSine * 0.45f;
                float voiceR = subSine * 0.45f;

                for (int u = 0; u < unisonCount; ++u)
                {
                    float detuneRatio = std::pow (2.0f, unisonOffsets[static_cast<size_t> (u)] * maxDetuneSemis);
                    double dt = (baseFreq * detuneRatio) * invSr;
                    double p  = dronePhases[static_cast<size_t> (v)][static_cast<size_t> (u)] + dt;
                    if (p >= 1.0) p -= 1.0;
                    dronePhases[static_cast<size_t> (v)][static_cast<size_t> (u)] = p;

                    float saw = static_cast<float> (2.0 * p - 1.0) - polyBlep (p, dt);
                    float tri = 1.0f - 4.0f * static_cast<float> (std::abs (std::round (p - 0.25) - (p - 0.25)));

                    float oscSample = (1.0f - curX) * (0.65f * tri + 0.35f * saw) + curX * saw;

                    float pan = unisonPans[static_cast<size_t> (u)] * stereoWidth;
                    float gL  = unisonGains[static_cast<size_t> (u)] * (0.5f * (1.0f - pan));
                    float gR  = unisonGains[static_cast<size_t> (u)] * (0.5f * (1.0f + pan));

                    voiceL += oscSample * gL;
                    voiceR += oscSample * gR;
                }

                synthL += voiceL * weight * dGain;
                synthR += voiceR * weight * dGain;
            }
        }

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

            float shapedEnv = mv.env * mv.env * (3.0f - 2.0f * mv.env);

            double subDt = (mv.freq * 0.5) * invSr;
            mv.subPhase += subDt;
            if (mv.subPhase >= 1.0) mv.subPhase -= 1.0;
            float subOut = static_cast<float> (std::sin (mv.subPhase * juce::MathConstants<double>::twoPi)) * 0.38f;

            float mL = subOut;
            float mR = subOut;

            for (int u = 0; u < unisonCount; ++u)
            {
                float detuneRatio = std::pow (2.0f, unisonOffsets[static_cast<size_t> (u)] * maxDetuneSemis);
                double dt = (mv.freq * detuneRatio) * invSr;
                double p  = mv.phases[static_cast<size_t> (u)] + dt;
                if (p >= 1.0) p -= 1.0;
                mv.phases[static_cast<size_t> (u)] = p;

                float saw = static_cast<float> (2.0 * p - 1.0) - polyBlep (p, dt);
                float tri = 1.0f - 4.0f * static_cast<float> (std::abs (std::round (p - 0.25) - (p - 0.25)));
                float osc = (1.0f - curX) * (0.6f * tri + 0.4f * saw) + curX * saw;

                float pan = unisonPans[static_cast<size_t> (u)] * stereoWidth;
                mL += osc * unisonGains[static_cast<size_t> (u)] * (0.5f * (1.0f - pan));
                mR += osc * unisonGains[static_cast<size_t> (u)] * (0.5f * (1.0f + pan));
            }

            synthL += mL * shapedEnv * 0.48f;
            synthR += mR * shapedEnv * 0.48f;
        }

        float driveGain = 1.15f + warmDrive * 3.8f;
        synthL = std::tanh (synthL * driveGain) / std::tanh (driveGain);
        synthR = std::tanh (synthR * driveGain) / std::tanh (driveGain);

        float cutoffHz = 140.0f * std::pow (48.0f, curY);
        droneFilterL1.setCutoffFrequency (cutoffHz);
        droneFilterL2.setCutoffFrequency (cutoffHz);
        droneFilterR1.setCutoffFrequency (cutoffHz);
        droneFilterR2.setCutoffFrequency (cutoffHz);

        float filtL = droneFilterL2.processSample (0, droneFilterL1.processSample (0, synthL));
        float filtR = droneFilterR2.processSample (0, droneFilterR1.processSample (0, synthR));

        filtL = dcBlockL.processSample (0, filtL);
        filtR = dcBlockR.processSample (0, filtR);

        // Stereo BBD Chorus
        chorusBufL[static_cast<size_t> (chorusWriteIdx)] = filtL;
        chorusBufR[static_cast<size_t> (chorusWriteIdx)] = filtR;

        chorusLfoPhase += 0.42 * invSr;
        if (chorusLfoPhase >= 1.0) chorusLfoPhase -= 1.0;

        float lfo1 = static_cast<float> (std::sin (chorusLfoPhase * juce::MathConstants<double>::twoPi));
        float lfo2 = static_cast<float> (std::cos (chorusLfoPhase * juce::MathConstants<double>::twoPi));

        auto readChorusDelay = [&] (const std::vector<float>& buf, float delayMs) {
            float delaySamples = (delayMs * 0.001f) * static_cast<float> (currentSampleRate);
            float readPos = static_cast<float> (chorusWriteIdx) - delaySamples;
            while (readPos < 0.0f) readPos += static_cast<float> (chorusBufSize);
            int idx0 = static_cast<int> (readPos) % chorusBufSize;
            int idx1 = (idx0 + 1) % chorusBufSize;
            float frac = readPos - std::floor (readPos);
            return buf[static_cast<size_t> (idx0)] * (1.0f - frac) + buf[static_cast<size_t> (idx1)] * frac;
        };

        float chorusedL = 0.72f * filtL + 0.38f * readChorusDelay (chorusBufR, 14.0f + lfo1 * 4.5f);
        float chorusedR = 0.72f * filtR + 0.38f * readChorusDelay (chorusBufL, 17.5f + lfo2 * 4.5f);

        chorusWriteIdx = (chorusWriteIdx + 1) % chorusBufSize;

        if (s % 4 == 0)
        {
            scopeSamples[static_cast<size_t> (scopeWriteIdx)].store (
                juce::jlimit (-1.0f, 1.0f, (chorusedL + chorusedR) * 1.15f),
                std::memory_order_relaxed);
            scopeWriteIdx = (scopeWriteIdx + 1) % scopeSize;
        }

        // ----------------------------------------------------------------------
        // PHASE 1: VALHALLA SUPERMASSIVE-CLASS 8-CHANNEL FDN CLOUD & SHIMMER
        // ----------------------------------------------------------------------
        float tankInL = dryL + chorusedL * 0.74f;
        float tankInR = dryR + chorusedR * 0.74f;

        float effDecay   = juce::jlimit (0.0f, 1.0f, decayAmt + 0.12f * prayerVal);
        float effShimmer = juce::jlimit (0.0f, 1.0f, shimmerAmt + 0.24f * prayerVal);
        float effDamp    = juce::jlimit (0.0f, 1.0f, spaceDamp + 0.25f * prayerVal);

        float wetL = 0.0f;
        float wetR = 0.0f;
        supermassiveTank.processSample (tankInL, tankInR, wetL, wetR,
                                        effDecay, spaceWarp, spaceMod,
                                        effShimmer, effDamp, stereoWidth);

        float duckGain = 1.0f - juce::jlimit (0.0f, 0.72f, inputEnvelope * autoDuck * 4.5f);
        float finalWetMix = juce::jlimit (0.0f, 1.0f, ambMix + 0.25f * prayerVal) * duckGain;

        outL[s] = dryL + chorusedL * 0.72f + wetL * finalWetMix * 1.25f;
        outR[s] = dryR + chorusedR * 0.72f + wetR * finalWetMix * 1.25f;
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