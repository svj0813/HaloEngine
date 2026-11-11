    #include "PluginProcessor.h"
#include "PluginEditor.h"

PluginEditor::PluginEditor (HaloEngineAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Define the initial size of the plugin window
    setSize (400, 300);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // Fill the background with black
    g.fillAll (juce::Colours::black);

    // Draw some placeholder text in white
    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawFittedText ("Halo Ambient Engine", getLocalBounds(), juce::Justification::centred, 1);
}

void PluginEditor::resized()
{
    // We will position our knobs and buttons here later
}