#include "PluginProcessor.h"
#include "PluginEditor.h"

HaloEngineAudioProcessor::HaloEngineAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

HaloEngineAudioProcessor::~HaloEngineAudioProcessor() {}

const juce::String HaloEngineAudioProcessor::getName() const { return JucePlugin_Name; }
bool HaloEngineAudioProcessor::acceptsMidi() const { return true; }
bool HaloEngineAudioProcessor::producesMidi() const { return false; }
bool HaloEngineAudioProcessor::isMidiEffect() const { return false; }
double HaloEngineAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int HaloEngineAudioProcessor::getNumPrograms() { return 1; }
int HaloEngineAudioProcessor::getCurrentProgram() { return 0; }
void HaloEngineAudioProcessor::setCurrentProgram (int index) {}
const juce::String HaloEngineAudioProcessor::getProgramName (int index) { return {}; }
void HaloEngineAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

void HaloEngineAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // We will initialize our Reverb and Synth here later
}

void HaloEngineAudioProcessor::releaseResources()
{
    // Free up memory when playback stops
}

bool HaloEngineAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void HaloEngineAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear output channels we aren't using to prevent feedback/noise
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Our DSP Math will go here
}

bool HaloEngineAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* HaloEngineAudioProcessor::createEditor() { return new PluginEditor (*this); }

void HaloEngineAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {}
void HaloEngineAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HaloEngineAudioProcessor();
}   