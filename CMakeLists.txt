cmake_minimum_required(VERSION 3.22)

project(HaloEngine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(/home/shauryaj/AudioDev/JUCE JUCE)

juce_add_plugin(HaloEngine
    COMPANY_NAME "ShawarmaAudio"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Shwr
    PLUGIN_CODE Halo
    FORMATS VST3 Standalone
    PRODUCT_NAME "Halo Ambient Engine"
)

juce_generate_juce_header(HaloEngine)

target_sources(HaloEngine
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/DSP/SupermassiveTank.h
        Source/DSP/WavetableOsc.h
        Source/DSP/ModFilterEngine.h
        Source/DSP/PresetBank.h
        Source/DSP/SetlistManager.h
)

target_compile_definitions(HaloEngine
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(HaloEngine
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
