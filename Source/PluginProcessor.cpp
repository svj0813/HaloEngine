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

    for (size_t v = 0; v < numVoices; ++v)
    {
        droneSubPhases[v] = dist (rng);
        for (size_t u = 0; u < unisonCount; ++u)
        {
            dronePhasesA[v][u] = dist (rng);
            dronePhasesB[v][u] = dist (rng);
        }
    }

    for (auto& mv : midiVoices)
    {
        mv.subPhase = dist (rng);
        for (size_t u = 0; u < unisonCount; ++u)
        {
            mv.phasesA[u] = dist (rng);
            mv.phasesB[u] = dist (rng);
        }
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
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("MORPH_Y", "Brightness Morph Y", 0.0f, 1.0f, 0.52f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("SHIMMER_AMT", "Shimmer Octave", 0.0f, 1.0f, 0.48f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("AMBIENCE_DECAY", "Space Decay", 0.0f, 1.0f, 0.86f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("AUTO_DUCK", "Band Auto-Duck", 0.0f, 1.0f, 0.40f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("AMBIENCE_MIX", "Ambience Mix", 0.0f, 1.0f, 0.58f));

    params.push_back (std::make_unique<juce::AudioParameterBool>   ("PRAYER_MACRO", "Prayer / Transition", false));
    params.push_back (std::make_unique<juce::AudioParameterBool>   ("KEY_DETECT_ON", "Smart Key Assist", true));

    // Phase 1: Supermassive FDN Space Parameters
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("SPACE_WARP", "Cloud Warp", 0.0f, 1.0f, 0.74f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("SPACE_MOD",  "Tank Mod",   0.0f, 1.0f, 0.55f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("SPACE_DAMP", "Air Damp",   0.0f, 1.0f, 0.42f));

    // Phase 2: Serum/Vital Dual Wavetable + Warp + Organic Air
    juce::StringArray wtNames { "ANALOG SAW-TRI", "VOCAL CHOIR", "GLASS BELL", "PWM ENSEMBLE", "BOWED STRINGS" };
    juce::StringArray warpModes { "OFF", "PHASE BEND", "HARD SYNC", "FM (B -> A)" };

    params.push_back (std::make_unique<juce::AudioParameterChoice> ("OSC_A_TABLE", "Osc A Table", wtNames, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("OSC_A_POS",   "Osc A Position", 0.0f, 1.0f, 0.45f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("OSC_A_WARP_MODE", "Osc A Warp Mode", warpModes, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("OSC_A_WARP",  "Osc A Warp",  0.0f, 1.0f, 0.22f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("OSC_A_LEVEL", "Osc A Level", 0.0f, 1.0f, 0.85f));

    params.push_back (std::make_unique<juce::AudioParameterChoice> ("OSC_B_TABLE", "Osc B Table", wtNames, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("OSC_B_POS",   "Osc B Position", 0.0f, 1.0f, 0.55f));
    params.push_back (std::make_unique<juce::AudioParameterInt>    ("OSC_B_OCT",   "Osc B Octave", -1, 2, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("OSC_B_LEVEL", "Osc B Level", 0.0f, 1.0f, 0.58f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("AIR_NOISE",   "Organic Air", 0.0f, 1.0f, 0.24f));

    // Phase 3: ZDF Multi-Mode Filter + ENV 2 + LFO 1 & LFO 2 Modulation Matrix
    juce::StringArray filterModes { "LP 24dB LADDER", "LP 12dB OB-6", "BP FORMANT", "NOTCH PHASER" };
    juce::StringArray lfoShapes   { "SINE", "TRIANGLE", "WANDER" };

    params.push_back (std::make_unique<juce::AudioParameterChoice> ("FILTER_MODE", "Filter Mode", filterModes, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("FILTER_ENV_AMT", "Filter Env 2", -1.0f, 1.0f, 0.35f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("ENV2_ATTACK",  "Env2 Attack",  0.01f, 3.0f, 0.65f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("ENV2_DECAY",   "Env2 Decay",   0.05f, 4.0f, 1.40f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("ENV2_SUSTAIN", "Env2 Sustain", 0.0f,  1.0f, 0.55f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("ENV2_RELEASE", "Env2 Release", 0.05f, 4.0f, 1.50f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("LFO1_RATE",    "LFO 1 Rate",   0.05f, 8.0f, 0.35f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("LFO1_SHAPE",   "LFO 1 Shape",  lfoShapes, 2));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("LFO1_WT_AMT",  "LFO 1 -> WT",  0.0f,  1.0f, 0.28f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("LFO2_RATE",    "LFO 2 Rate",   0.05f, 8.0f, 0.18f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("LFO2_SHAPE",   "LFO 2 Shape",  lfoShapes, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>  ("LFO2_CUT_AMT", "LFO 2 -> Cut", 0.0f,  1.0f, 0.22f));

    // 6-Voice Harmonic Mixer & Global Synth Controls
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_SUB",   "Sub Octave",  0.0f, 1.0f, 0.80f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_ROOT",  "Root Voice",  0.0f, 1.0f, 0.85f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_FIFTH", "Fifth Voice", 0.0f, 1.0f, 0.52f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_OCT",   "Octave Voice",0.0f, 1.0f, 0.48f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_NINTH", "Ninth Voice", 0.0f, 1.0f, 0.34f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOICE_HIGH",  "High Fifth",  0.0f, 1.0f, 0.22f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_DETUNE", "Unison Detune", 0.0f, 1.0f, 0.42f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_WIDTH",  "Stereo Width",  0.0f, 1.0f, 0.90f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SYNTH_RES",    "Filter Res",    0.0f, 1.0f, 0.24f));
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

    for (size_t i = 0; i < numVoices; ++i)
        voiceFreqSmoothed[i].reset (sampleRate, glideSec);

    for (auto& mv : midiVoices)
    {
        mv.env1Amp.prepare (sampleRate);
        mv.env2Mod.prepare (sampleRate);
    }

    lfo1.prepare (sampleRate);
    lfo2.prepare (sampleRate);
    mainMultiFilter.prepare (sampleRate);

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

    organicAir.prepare (sampleRate);
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

    for (size_t i = 0; i < numVoices; ++i)
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
            size_t slot = 0;
            float lowestEnv = 10.0f;
            for (size_t v = 0; v < maxMidiVoices; ++v)
            {
                if (! midiVoices[v].env1Amp.isActive())
                {
                    slot = v;
                    break;
                }
                if (midiVoices[v].env1Amp.getValue() < lowestEnv)
                {
                    lowestEnv = midiVoices[v].env1Amp.getValue();
                    slot = v;
                }
            }
            auto& mv = midiVoices[slot];
            mv.midiNote = note;
            mv.freq = static_cast<float> (440.0 * std::pow (2.0, (note - 69.0) / 12.0));
            mv.env1Amp.noteOn();
            mv.env2Mod.noteOn();
        }
        else if (msg.isNoteOff())
        {
            int note = msg.getNoteNumber();
            for (auto& mv : midiVoices)
            {
                if (mv.env1Amp.isActive() && mv.midiNote == note)
                {
                    mv.env1Amp.noteOff();
                    mv.env2Mod.noteOff();
                }
            }
        }
        else if (msg.isAllNotesOff())
        {
            for (auto& mv : midiVoices)
            {
                mv.env1Amp.noteOff();
                mv.env2Mod.noteOff();
            }
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

    int   oscATable    = static_cast<int> (*apvts.getRawParameterValue ("OSC_A_TABLE"));
    float oscAPosBase  = *apvts.getRawParameterValue ("OSC_A_POS");
    int   oscAWarpMode = static_cast<int> (*apvts.getRawParameterValue ("OSC_A_WARP_MODE"));
    float oscAWarpAmt  = *apvts.getRawParameterValue ("OSC_A_WARP");
    float oscALevel    = *apvts.getRawParameterValue ("OSC_A_LEVEL");

    int   oscBTable    = static_cast<int> (*apvts.getRawParameterValue ("OSC_B_TABLE"));
    float oscBPosBase  = *apvts.getRawParameterValue ("OSC_B_POS");
    int   oscBOct      = static_cast<int> (*apvts.getRawParameterValue ("OSC_B_OCT"));
    float oscBLevel    = *apvts.getRawParameterValue ("OSC_B_LEVEL");
    float oscBOctMult  = std::pow (2.0f, static_cast<float> (oscBOct));

    float airNoiseAmt  = *apvts.getRawParameterValue ("AIR_NOISE");

    int   filterMode   = static_cast<int> (*apvts.getRawParameterValue ("FILTER_MODE"));
    float filterEnvAmt = *apvts.getRawParameterValue ("FILTER_ENV_AMT");
    float env2Att      = *apvts.getRawParameterValue ("ENV2_ATTACK");
    float env2Dec      = *apvts.getRawParameterValue ("ENV2_DECAY");
    float env2Sus      = *apvts.getRawParameterValue ("ENV2_SUSTAIN");
    float env2Rel      = *apvts.getRawParameterValue ("ENV2_RELEASE");

    float lfo1Rate     = *apvts.getRawParameterValue ("LFO1_RATE");
    int   lfo1Shape    = static_cast<int> (*apvts.getRawParameterValue ("LFO1_SHAPE"));
    float lfo1WtAmt    = *apvts.getRawParameterValue ("LFO1_WT_AMT");

    float lfo2Rate     = *apvts.getRawParameterValue ("LFO2_RATE");
    int   lfo2Shape    = static_cast<int> (*apvts.getRawParameterValue ("LFO2_SHAPE"));
    float lfo2CutAmt   = *apvts.getRawParameterValue ("LFO2_CUT_AMT");

    std::array<float, numVoices> voiceWeights {
        *apvts.getRawParameterValue ("VOICE_SUB")   * 0.50f,
        *apvts.getRawParameterValue ("VOICE_ROOT")  * 0.46f,
        *apvts.getRawParameterValue ("VOICE_FIFTH") * 0.35f,
        *apvts.getRawParameterValue ("VOICE_OCT")   * 0.33f,
        *apvts.getRawParameterValue ("VOICE_NINTH") * 0.26f,
        *apvts.getRawParameterValue ("VOICE_HIGH")  * 0.21f
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
        for (size_t i = 0; i < numVoices; ++i)
            voiceFreqSmoothed[i].reset (currentSampleRate, glideSec);
        lastGlideSec = glideSec;
    }

    if (keyIndex != lastKeyIndex)
    {
        updateDroneFrequencies (keyIndex, false);
        lastKeyIndex = keyIndex;
    }

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
    const int chorusBufSize = static_cast<int> (chorusBufL.size());
    const double invSr      = 1.0 / currentSampleRate;

    static constexpr std::array<float, unisonCount> unisonOffsets { -1.0f, -0.58f, -0.22f, 0.0f, 0.22f, 0.58f, 1.0f };
    static constexpr std::array<float, unisonCount> unisonPans    { -0.95f, 0.78f, -0.45f, 0.0f, 0.45f, -0.78f, 0.95f };
    static constexpr std::array<float, unisonCount> unisonGains   {  0.22f, 0.28f,  0.34f, 0.42f, 0.34f, 0.28f, 0.22f };

    float lastModPosA = oscAPosBase;
    float lastModPosB = oscBPosBase;
    float lastModCut  = morphY;

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

        // Advance LFO 1 (Wavetable Position Scanner) & LFO 2 (Filter Cutoff Drift)
        float lfo1Val = lfo1.process (lfo1Rate, lfo1Shape);
        float lfo2Val = lfo2.process (lfo2Rate, lfo2Shape);

        float effPosA = juce::jlimit (0.0f, 1.0f, oscAPosBase * 0.5f + curX * 0.5f + lfo1Val * lfo1WtAmt * 0.42f);
        float effPosB = juce::jlimit (0.0f, 1.0f, oscBPosBase * 0.5f + curX * 0.5f - lfo1Val * lfo1WtAmt * 0.35f);
        float weightA = oscALevel * (1.15f - curX * 0.45f);
        float weightB = oscBLevel * (0.55f + curX * 0.65f);

        float maxDetuneSemis = (0.04f + detuneAmt * 0.36f) / 12.0f;

        float synthL = 0.0f;
        float synthR = 0.0f;
        float activeEnergy = 0.0f;
        float maxEnv2Mod = 0.0f;

        if (dGain > 0.0005f)
        {
            activeEnergy += dGain * 0.65f;
            for (size_t v = 0; v < numVoices; ++v)
            {
                float baseFreq = voiceFreqSmoothed[v].getNextValue();
                float weight = voiceWeights[v] * (v == 0 ? (1.0f + 0.45f * prayerVal) : 1.0f);
                if (weight < 0.001f)
                    continue;

                double subDt = baseFreq * invSr;
                double subP  = droneSubPhases[v] + subDt;
                if (subP >= 1.0) subP -= 1.0;
                droneSubPhases[v] = subP;
                float subSine = static_cast<float> (std::sin (subP * juce::MathConstants<double>::twoPi));

                float voiceL = subSine * 0.42f;
                float voiceR = subSine * 0.42f;

                for (size_t u = 0; u < unisonCount; ++u)
                {
                    float detuneRatio = std::pow (2.0f, unisonOffsets[u] * maxDetuneSemis);

                    double dtB = (baseFreq * oscBOctMult * detuneRatio) * invSr;
                    double pB  = dronePhasesB[v][u] + dtB;
                    if (pB >= 1.0) pB -= 1.0;
                    dronePhasesB[v][u] = pB;
                    float sampleB = wavetableBank.lookup (oscBTable, effPosB, pB);

                    double dtA = (baseFreq * detuneRatio) * invSr;
                    double pA  = dronePhasesA[v][u] + dtA;
                    if (pA >= 1.0) pA -= 1.0;
                    dronePhasesA[v][u] = pA;
                    double warpedPhaseA = applySpectralWarp (pA, oscAWarpMode, oscAWarpAmt, sampleB);
                    float sampleA = wavetableBank.lookup (oscATable, effPosA, warpedPhaseA);

                    float combinedOsc = (sampleA * weightA + sampleB * weightB) * 0.65f;

                    float pan = unisonPans[u] * stereoWidth;
                    float gL  = unisonGains[u] * (0.5f * (1.0f - pan));
                    float gR  = unisonGains[u] * (0.5f * (1.0f + pan));

                    voiceL += combinedOsc * gL;
                    voiceR += combinedOsc * gR;
                }

                synthL += voiceL * weight * dGain;
                synthR += voiceR * weight * dGain;
            }
        }

        for (size_t m = 0; m < maxMidiVoices; ++m)
        {
            auto& mv = midiVoices[m];
            if (! mv.env1Amp.isActive())
                continue;

            float shapedEnv = mv.env1Amp.process (attackSec, 0.35f, 1.0f, releaseSec);
            float modEnvVal = mv.env2Mod.process (env2Att, env2Dec, env2Sus, env2Rel);
            maxEnv2Mod = juce::jmax (maxEnv2Mod, modEnvVal);
            activeEnergy += shapedEnv * 0.45f;

            double subDt = (mv.freq * 0.5) * invSr;
            mv.subPhase += subDt;
            if (mv.subPhase >= 1.0) mv.subPhase -= 1.0;
            float subOut = static_cast<float> (std::sin (mv.subPhase * juce::MathConstants<double>::twoPi)) * 0.36f;

            float mL = subOut;
            float mR = subOut;

            for (size_t u = 0; u < unisonCount; ++u)
            {
                float detuneRatio = std::pow (2.0f, unisonOffsets[u] * maxDetuneSemis);

                double dtB = (mv.freq * oscBOctMult * detuneRatio) * invSr;
                double pB  = mv.phasesB[u] + dtB;
                if (pB >= 1.0) pB -= 1.0;
                mv.phasesB[u] = pB;
                float sampleB = wavetableBank.lookup (oscBTable, effPosB, pB);

                double dtA = (mv.freq * detuneRatio) * invSr;
                double pA  = mv.phasesA[u] + dtA;
                if (pA >= 1.0) pA -= 1.0;
                mv.phasesA[u] = pA;
                double warpedPhaseA = applySpectralWarp (pA, oscAWarpMode, oscAWarpAmt, sampleB);
                float sampleA = wavetableBank.lookup (oscATable, effPosA, warpedPhaseA);

                float combinedOsc = (sampleA * weightA + sampleB * weightB) * 0.65f;

                float pan = unisonPans[u] * stereoWidth;
                mL += combinedOsc * unisonGains[u] * (0.5f * (1.0f - pan));
                mR += combinedOsc * unisonGains[u] * (0.5f * (1.0f + pan));
            }

            synthL += mL * shapedEnv * 0.48f;
            synthR += mR * shapedEnv * 0.48f;
        }

        float airL = 0.0f, airR = 0.0f;
        organicAir.processSample (airL, airR, curY, airNoiseAmt * juce::jmin (1.0f, activeEnergy));
        synthL += airL;
        synthR += airR;

        // Pre-Filter Analog Drive
        float driveGain = 1.15f + warmDrive * 3.8f;
        synthL = std::tanh (synthL * driveGain) / std::tanh (driveGain);
        synthR = std::tanh (synthR * driveGain) / std::tanh (driveGain);

        // Modulated Cutoff: Base Y + ENV 2 Swell + LFO 2 Drift
        float modCutoffNorm = juce::jlimit (0.0f, 1.0f,
            curY + maxEnv2Mod * filterEnvAmt * 0.48f + lfo2Val * lfo2CutAmt * 0.35f);
        float cutoffHz = 130.0f * std::pow (54.0f, modCutoffNorm);

        mainMultiFilter.processStereo (synthL, synthR, cutoffHz, filterRes, filterMode);

        float filtL = dcBlockL.processSample (0, synthL);
        float filtR = dcBlockR.processSample (0, synthR);

        // Stereo BBD Chorus
        chorusBufL[static_cast<size_t> (chorusWriteIdx)] = filtL;
        chorusBufR[static_cast<size_t> (chorusWriteIdx)] = filtR;

        chorusLfoPhase += 0.42 * invSr;
        if (chorusLfoPhase >= 1.0) chorusLfoPhase -= 1.0;

        float lfo1C = static_cast<float> (std::sin (chorusLfoPhase * juce::MathConstants<double>::twoPi));
        float lfo2C = static_cast<float> (std::cos (chorusLfoPhase * juce::MathConstants<double>::twoPi));

        auto readChorusDelay = [&] (const std::vector<float>& buf, float delayMs) {
            float delaySamples = (delayMs * 0.001f) * static_cast<float> (currentSampleRate);
            float readPos = static_cast<float> (chorusWriteIdx) - delaySamples;
            while (readPos < 0.0f) readPos += static_cast<float> (chorusBufSize);
            int idx0 = static_cast<int> (readPos) % chorusBufSize;
            int idx1 = (idx0 + 1) % chorusBufSize;
            float frac = readPos - std::floor (readPos);
            return buf[static_cast<size_t> (idx0)] * (1.0f - frac) + buf[static_cast<size_t> (idx1)] * frac;
        };

        float chorusedL = 0.72f * filtL + 0.38f * readChorusDelay (chorusBufR, 14.0f + lfo1C * 4.5f);
        float chorusedR = 0.72f * filtR + 0.38f * readChorusDelay (chorusBufL, 17.5f + lfo2C * 4.5f);

        chorusWriteIdx = (chorusWriteIdx + 1) % chorusBufSize;

        if (s % 4 == 0)
        {
            scopeSamples[static_cast<size_t> (scopeWriteIdx)].store (
                juce::jlimit (-1.0f, 1.0f, (chorusedL + chorusedR) * 1.15f),
                std::memory_order_relaxed);
            scopeWriteIdx = (scopeWriteIdx + 1) % scopeSize;
        }

        lastModPosA = effPosA;
        lastModPosB = effPosB;
        lastModCut  = modCutoffNorm;

        // Valhalla Supermassive-Class FDN Cloud & Shimmer
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

    liveOscAPos.store (lastModPosA, std::memory_order_relaxed);
    liveOscBPos.store (lastModPosB, std::memory_order_relaxed);
    liveFilterNorm.store (lastModCut, std::memory_order_relaxed);

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
