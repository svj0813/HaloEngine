#pragma once
#include <JuceHeader.h>
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

    // Shared MIDI keyboard state for clickable Synth Tab keyboard & external MIDI
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

    double currentSampleRate = 44100.0;
    int lastKeyIndex = -1;
    float lastGlideSec = 0.80f;

    // 6-Voice Harmonic Drone Engine (Sub, Root, 5th, Oct, 9th, High 5th)
    static constexpr int numVoices = 6;
    std::array<juce::dsp::Oscillator<float>, numVoices> warmOscs;
    std::array<juce::dsp::Oscillator<float>, numVoices> choirOscs;
    std::array<juce::dsp::Oscillator<float>, numVoices> detuneOscs;
    std::array<juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative>, numVoices> voiceFreqSmoothed;

    // Polyphonic MIDI Synth Voices (8-voice playable layer from Synth Tab or MIDI controller)
    struct MidiVoice
    {
        bool active = false;
        int midiNote = -1;
        float freq = 261.63f;
        double phase1 = 0.0;
        double phase2 = 0.0;
        float env = 0.0f;
        bool releasing = false;
    };
    static constexpr int maxMidiVoices = 8;
    std::array<MidiVoice, maxMidiVoices> midiVoices;

    juce::dsp::StateVariableTPTFilter<float> droneFilterL, droneFilterR;
    juce::SmoothedValue<float> droneGainSmoothed;
    juce::SmoothedValue<float> prayerMacroSmoothed;
    juce::SmoothedValue<float> morphXSmoothed;
    juce::SmoothedValue<float> morphYSmoothed;

    // Shimmer & Lush Stereo Ambience Engine
    juce::Reverb stereoReverb;
    juce::Reverb::Parameters reverbParams;
    juce::dsp::StateVariableTPTFilter<float> shimmerFilterL, shimmerFilterR;

    // Granular Dual-Head Octave-Up Shimmer Buffer
    std::vector<float> pitchBufferL, pitchBufferR;
    int pitchWriteIdx = 0;
    float pitchReadPhase = 0.0f;

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