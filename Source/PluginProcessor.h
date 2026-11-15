#pragma once
#include <JuceHeader.h>
#include "DSP/SupermassiveTank.h"
#include "DSP/WavetableOsc.h"
#include "DSP/ModFilterEngine.h"
#include <array>
#include <vector>
#include <atomic>

class HaloEngineAudioProcessor : public juce::AudioProcessor
{
public:
    HaloEngineAudioProcessor();
    ~HaloEngineAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::MidiKeyboardState keyboardState;
    WavetableBank wavetableBank;

    std::atomic<int>   suggestedKeyIndex { -1 };
    std::atomic<float> outLevelL { 0.0f };
    std::atomic<float> outLevelR { 0.0f };
    std::atomic<float> duckMeter { 0.0f };

    // Live telemetry for Synth Tab UI (Modulated WT Positions & Oscilloscope)
    std::atomic<float> liveOscAPos { 0.45f };
    std::atomic<float> liveOscBPos { 0.55f };
    std::atomic<float> liveFilterNorm { 0.52f };

    static constexpr int scopeSize = 128;
    std::array<std::atomic<float>, scopeSize> scopeSamples {};

private:
    void updateDroneFrequencies (int keyIndex, bool snapImmediately);
    void analyzeIncomingPitch();

    static inline double applySpectralWarp (double phase, int warpMode, float warpAmt, float modSignal) noexcept
    {
        if (warpAmt <= 0.001f || warpMode == 0)
            return phase;

        if (warpMode == 1)
        {
            double pivot = 0.5 - warpAmt * 0.42;
            if (phase < pivot)
                return 0.5 * (phase / pivot);
            return 0.5 + 0.5 * ((phase - pivot) / (1.0 - pivot));
        }
        if (warpMode == 2)
        {
            double syncMult = 1.0 + warpAmt * 2.5;
            double synced = phase * syncMult;
            return synced - std::floor (synced);
        }
        double fmPhase = phase + static_cast<double> (modSignal * warpAmt * 0.45f);
        return fmPhase - std::floor (fmPhase);
    }

    double currentSampleRate = 44100.0;
    int lastKeyIndex = -1;
    float lastGlideSec = 0.80f;

    static constexpr size_t numVoices = 6;
    static constexpr size_t unisonCount = 7;

    std::array<std::array<double, unisonCount>, numVoices> dronePhasesA {};
    std::array<std::array<double, unisonCount>, numVoices> dronePhasesB {};
    std::array<double, numVoices> droneSubPhases {};
    std::array<juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative>, numVoices> voiceFreqSmoothed;

    struct MidiVoice
    {
        int midiNote = -1;
        float freq = 261.63f;
        double subPhase = 0.0;
        std::array<double, unisonCount> phasesA {};
        std::array<double, unisonCount> phasesB {};
        HaloADSR env1Amp;
        HaloADSR env2Mod;
    };
    static constexpr size_t maxMidiVoices = 8;
    std::array<MidiVoice, maxMidiVoices> midiVoices;

    // Phase 3: Modulation LFOs & ZDF Multi-Mode Filter
    HaloLFO lfo1 { 101 };
    HaloLFO lfo2 { 202 };
    HaloMultiFilter mainMultiFilter;

    OrganicAirGenerator organicAir;
    juce::dsp::StateVariableTPTFilter<float> dcBlockL, dcBlockR;

    juce::SmoothedValue<float> droneGainSmoothed;
    juce::SmoothedValue<float> prayerMacroSmoothed;
    juce::SmoothedValue<float> morphXSmoothed;
    juce::SmoothedValue<float> morphYSmoothed;

    std::vector<float> chorusBufL, chorusBufR;
    int chorusWriteIdx = 0;
    double chorusLfoPhase = 0.0;

    SupermassiveTank supermassiveTank;

    float inputEnvelope = 0.0f;

    std::vector<float> detectBuffer;
    int detectWritePos = 0;
    int candidateKey = -1;
    int candidateConfirmCount = 0;
    int scopeWriteIdx = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HaloEngineAudioProcessor)
};
