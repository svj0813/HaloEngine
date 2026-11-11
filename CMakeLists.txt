cmake_minimum_required(VERSION 3.22)
project(HaloEngine VERSION 1.0.0)

# Tell CMake where to find the JUCE framework (one folder up)
add_subdirectory(../JUCE JUCE)

# Define the VST3 and Standalone plugin properties
juce_add_plugin(HaloEngine
    COMPANY_NAME "WorshipDSP"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Wdsp
    PLUGIN_CODE Halo
    FORMATS VST3 Standalone
    VST2_CATEGORY kPlugCategEffect
    PRODUCT_NAME "Halo Ambient Engine")

# Tell CMake which C++ files to compile (we will make these next)
target_sources(HaloEngine PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h)

# Use modern C++20 standard
target_compile_features(HaloEngine PRIVATE cxx_std_20)
target_compile_definitions(HaloEngine PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link the necessary JUCE modules (including the DSP module for our reverb/pitch math)
target_link_libraries(HaloEngine PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_audio_processors
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
    juce_generate_juce_header(HaloEngine)