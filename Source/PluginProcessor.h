#pragma once
#include <JuceHeader.h>
#include "DSP/SupermassiveTank.h"
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

    // Thread-safe UI telemetry & key detection atomics
    std::atomic<int>   suggestedKeyIndex { -1 };
    std::atomic<float> outLevelL { 0.0f };
    std::atomic<float> outLevelR { 0.0f };
    std::atomic<float> duckMeter { 0.0f };

    // Live oscilloscope ring buffer for Synth Tab
    static constexpr int scopeSize = 128;
    std::array<std::atomic<float>, scopeSize> scopeSamples {};

private:
    void updateDroneFrequencies (int keyIndex, bool snapImmediately);
    void analyzeIncomingPitch();

    static inline float polyBlep (double t, double dt) noexcept
    {
        if (t < dt)
        {
            t /= dt;
            return static_cast<float> (t + t - t * t - 1.0);
        }
        if (t > 1.0 - dt)
        {
            t = (t - 1.0) / dt;
            return static_cast<float> (t * t + t + t + 1.0);
        }
        return 0.0f;
    }

    double currentSampleRate = 44100.0;
    int lastKeyIndex = -1;
    float lastGlideSec = 0.80f;

    static constexpr int numVoices = 6;
    static constexpr int unisonCount = 7;

    std::array<std::array<double, unisonCount>, numVoices> dronePhases {};
    std::array<double, numVoices> droneSubPhases {};
    std::array<juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative>, numVoices> voiceFreqSmoothed;

    struct MidiVoice
    {
        bool active = false;
        bool releasing = false;
        int midiNote = -1;
        float freq = 261.63f;
        float env = 0.0f;
        double subPhase = 0.0;
        std::array<double, unisonCount> phases {};
    };
    static constexpr int maxMidiVoices = 8;
    std::array<MidiVoice, maxMidiVoices> midiVoices;

    juce::dsp::StateVariableTPTFilter<float> droneFilterL1, droneFilterL2;
    juce::dsp::StateVariableTPTFilter<float> droneFilterR1, droneFilterR2;
    juce::dsp::StateVariableTPTFilter<float> dcBlockL, dcBlockR;

    juce::SmoothedValue<float> droneGainSmoothed;
    juce::SmoothedValue<float> prayerMacroSmoothed;
    juce::SmoothedValue<float> morphXSmoothed;
    juce::SmoothedValue<float> morphYSmoothed;

    // 4-Tap Stereo BBD Ensemble Chorus
    std::vector<float> chorusBufL, chorusBufR;
    int chorusWriteIdx = 0;
    double chorusLfoPhase = 0.0;

    // Phase 1: Valhalla Supermassive-Class 8-Channel Modulated FDN Space Engine
    SupermassiveTank supermassiveTank;

    // Reactive Auto-Ducking Envelope Follower
    float inputEnvelope = 0.0f;

    // Non-Intrusive Autocorrelation Pitch / Key Detector
    std::vector<float> detectBuffer;
    int detectWritePos = 0;
    int candidateKey = -1;
    int candidateConfirmCount = 0;
    int scopeWriteIdx = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HaloEngineAudioProcessor)
};