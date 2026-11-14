#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>

class HaloLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HaloLookAndFeel();

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle, juce::Slider& slider) override;

    juce::Label* createSliderTextBox (juce::Slider& slider) override;

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;
};

class PanoramicHarmonicDisplay : public juce::Component
{
public:
    PanoramicHarmonicDisplay (HaloEngineAudioProcessor& proc);
    void paint (juce::Graphics& g) override;
    void advanceAnimation (float dt);

private:
    HaloEngineAudioProcessor& processor;
    float animPhase = 0.0f;
};

class SynthOscilloscope : public juce::Component
{
public:
    SynthOscilloscope (HaloEngineAudioProcessor& proc);
    void paint (juce::Graphics& g) override;

private:
    HaloEngineAudioProcessor& processor;
};

class VerticalMeterStrip : public juce::Component
{
public:
    VerticalMeterStrip (const juce::String& labelText, bool isGainReduction = false);
    void setLevels (float newL, float newR);
    void paint (juce::Graphics& g) override;

private:
    juce::String title;
    bool duckMode = false;
    float levelL = 0.0f;
    float levelR = 0.0f;
};

class HardwareKeybed : public juce::Component
{
public:
    HardwareKeybed (HaloEngineAudioProcessor& proc);
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

private:
    int hitTestNote (juce::Point<float> pos) const;
    juce::Rectangle<float> getNaturalRect (int whiteIdx) const;
    juce::Rectangle<float> getSharpRect (int sharpSlot) const;

    HaloEngineAudioProcessor& processor;
    const juce::StringArray noteNames { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
};

class HardwareDroneSwitch : public juce::Component
{
public:
    HardwareDroneSwitch (HaloEngineAudioProcessor& proc);
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;

private:
    HaloEngineAudioProcessor& processor;
    const juce::StringArray noteNames { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
};

class HarmonicXYPad : public juce::Component
{
public:
    HarmonicXYPad (HaloEngineAudioProcessor& proc);
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void advanceAnimation (float dt);

private:
    void updateFromPosition (juce::Point<float> pos);
    HaloEngineAudioProcessor& processor;
    float animPhase = 0.0f;
};

class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::Timer
{
public:
    PluginEditor (HaloEngineAudioProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void switchTab (int tabIndex);
    void configureKnob (juce::Slider& slider, juce::Label& label,
                        const juce::String& text, juce::Colour accent);
    void configureVerticalFader (juce::Slider& slider, juce::Label& label,
                                 const juce::String& text, juce::Colour accent);
    void applyPreset (int idx);

    HaloEngineAudioProcessor& audioProcessor;
    HaloLookAndFeel haloLookAndFeel;

    // Top Navigation Tabs & Preset Toolbar
    juce::TextButton tabMainBtn  { "STAGE VIEW" };
    juce::TextButton tabSynthBtn { "SYNTH ENGINE" };
    int activeTab = 0;

    juce::TextButton prevPresetBtn { "<" };
    juce::TextButton presetNameBtn { "01 // CATHEDRAL HALO" };
    juce::TextButton nextPresetBtn { ">" };
    int currentPresetIdx = 0;

    VerticalMeterStrip leftDuckMeter  { "DUCK", true };
    VerticalMeterStrip rightOutMeter  { "OUT",  false };

    // TAB 0: STAGE PERFORMANCE
    PanoramicHarmonicDisplay canopyDisplay;
    HardwareKeybed           keybed;
    HardwareDroneSwitch      droneSwitch;

    juce::TextButton suggestKeyBtn { "LOCKED: C" };
    int currentSuggestedKey = -1;

    juce::TextButton keyAssistToggle { "AUTO" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> keyAssistAttach;

    juce::Slider droneLevelSlider;
    juce::Label  droneLevelLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> droneLevelAttach;

    HarmonicXYPad xyPad;

    juce::Slider shimmerSlider, decaySlider, duckSlider, mixSlider;
    juce::Label  shimmerLabel, decayLabel, duckLabel, mixLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> shimmerAttach, decayAttach, duckAttach, mixAttach;

    juce::TextButton prayerMacroBtn { "PRAYER / SWELL" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> prayerMacroAttach;

    // TAB 1: SYNTH & VOICING ENGINE LAB
    SynthOscilloscope oscilloscope;

    std::array<juce::Slider, 6> voiceFaders;
    std::array<juce::Label,  6> voiceLabels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 6> voiceAttachments;

    juce::Slider detuneSlider, widthSlider, resSlider, driveSlider, glideSlider, attackSlider, releaseSlider;
    juce::Label  detuneLabel,  widthLabel,  resLabel,  driveLabel,  glideLabel,  attackLabel,  releaseLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        detuneAttach, widthAttach, resAttach, driveAttach, glideAttach, attackAttach, releaseAttach;

    juce::MidiKeyboardComponent synthKeyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};