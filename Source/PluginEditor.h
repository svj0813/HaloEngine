#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    PluginEditor (HaloEngineAudioProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    HaloEngineAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};