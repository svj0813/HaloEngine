#pragma once
#include <JuceHeader.h>
#include <vector>

struct HaloPreset
{
    juce::String name;
    juce::String category;
    std::vector<std::pair<juce::String, float>> values;
};

class HaloPresetBank
{
public:
    static const std::vector<HaloPreset>& getPresets()
    {
        static const std::vector<HaloPreset> presets = {
            {
                "01 // CRYSTAL HALO", "HIGH SHIMMER",
                {
                    { "DRONE_OCTAVE", 1.0f },
                    { "OSC_A_TABLE", 2.0f }, { "OSC_A_POS", 0.62f }, { "OSC_A_WARP_MODE", 1.0f }, { "OSC_A_WARP", 0.24f }, { "OSC_A_LEVEL", 0.85f },
                    { "OSC_B_TABLE", 1.0f }, { "OSC_B_POS", 0.68f }, { "OSC_B_OCT", 1.0f },       { "OSC_B_LEVEL", 0.78f }, { "AIR_NOISE", 0.32f },
                    { "FILTER_MODE", 1.0f }, { "MORPH_X", 0.62f },   { "MORPH_Y", 0.76f },       { "SYNTH_RES", 0.22f },   { "SYNTH_DRIVE", 0.16f },
                    { "FILTER_ENV_AMT", 0.35f }, { "LFO1_RATE", 0.42f }, { "LFO1_SHAPE", 0.0f }, { "LFO1_WT_AMT", 0.34f },
                    { "LFO2_RATE", 0.20f }, { "LFO2_SHAPE", 0.0f },  { "LFO2_CUT_AMT", 0.20f },
                    { "SHIMMER_AMT", 0.78f }, { "AMBIENCE_DECAY", 0.90f }, { "SPACE_WARP", 0.82f }, { "SPACE_MOD", 0.62f }, { "SPACE_DAMP", 0.20f }, { "AMBIENCE_MIX", 0.64f },
                    { "VOICE_SUB", 0.22f }, { "VOICE_ROOT", 0.68f }, { "VOICE_FIFTH", 0.74f }, { "VOICE_OCT", 0.88f }, { "VOICE_NINTH", 0.82f }, { "VOICE_HIGH", 0.78f },
                    { "SYNTH_DETUNE", 0.48f }, { "SYNTH_WIDTH", 0.96f }, { "SYNTH_ATTACK", 0.25f }, { "SYNTH_RELEASE", 1.65f }
                }
            },
            {
                "02 // ANGELIC GLASS", "ETHEREAL BELL",
                {
                    { "DRONE_OCTAVE", 2.0f },
                    { "OSC_A_TABLE", 2.0f }, { "OSC_A_POS", 0.78f }, { "OSC_A_WARP_MODE", 3.0f }, { "OSC_A_WARP", 0.28f }, { "OSC_A_LEVEL", 0.88f },
                    { "OSC_B_TABLE", 2.0f }, { "OSC_B_POS", 0.85f }, { "OSC_B_OCT", 2.0f },       { "OSC_B_LEVEL", 0.74f }, { "AIR_NOISE", 0.28f },
                    { "FILTER_MODE", 1.0f }, { "MORPH_X", 0.72f },   { "MORPH_Y", 0.84f },       { "SYNTH_RES", 0.26f },   { "SYNTH_DRIVE", 0.12f },
                    { "FILTER_ENV_AMT", 0.42f }, { "LFO1_RATE", 0.58f }, { "LFO1_SHAPE", 1.0f }, { "LFO1_WT_AMT", 0.38f },
                    { "LFO2_RATE", 0.28f }, { "LFO2_SHAPE", 0.0f },  { "LFO2_CUT_AMT", 0.22f },
                    { "SHIMMER_AMT", 0.88f }, { "AMBIENCE_DECAY", 0.93f }, { "SPACE_WARP", 0.88f }, { "SPACE_MOD", 0.70f }, { "SPACE_DAMP", 0.16f }, { "AMBIENCE_MIX", 0.68f },
                    { "VOICE_SUB", 0.12f }, { "VOICE_ROOT", 0.58f }, { "VOICE_FIFTH", 0.72f }, { "VOICE_OCT", 0.90f }, { "VOICE_NINTH", 0.86f }, { "VOICE_HIGH", 0.84f },
                    { "SYNTH_DETUNE", 0.52f }, { "SYNTH_WIDTH", 0.98f }, { "SYNTH_ATTACK", 0.12f }, { "SYNTH_RELEASE", 2.10f }
                }
            },
            {
                "03 // SUPERNOVA CHOIR", "VOCAL SHIMMER",
                {
                    { "DRONE_OCTAVE", 1.0f },
                    { "OSC_A_TABLE", 1.0f }, { "OSC_A_POS", 0.54f }, { "OSC_A_WARP_MODE", 1.0f }, { "OSC_A_WARP", 0.26f }, { "OSC_A_LEVEL", 0.90f },
                    { "OSC_B_TABLE", 4.0f }, { "OSC_B_POS", 0.72f }, { "OSC_B_OCT", 1.0f },       { "OSC_B_LEVEL", 0.76f }, { "AIR_NOISE", 0.36f },
                    { "FILTER_MODE", 1.0f }, { "MORPH_X", 0.64f },   { "MORPH_Y", 0.74f },       { "SYNTH_RES", 0.24f },   { "SYNTH_DRIVE", 0.16f },
                    { "FILTER_ENV_AMT", 0.38f }, { "LFO1_RATE", 0.32f }, { "LFO1_SHAPE", 0.0f }, { "LFO1_WT_AMT", 0.42f },
                    { "LFO2_RATE", 0.16f }, { "LFO2_SHAPE", 2.0f },  { "LFO2_CUT_AMT", 0.24f },
                    { "SHIMMER_AMT", 0.76f }, { "AMBIENCE_DECAY", 0.92f }, { "SPACE_WARP", 0.86f }, { "SPACE_MOD", 0.66f }, { "SPACE_DAMP", 0.24f }, { "AMBIENCE_MIX", 0.66f },
                    { "VOICE_SUB", 0.25f }, { "VOICE_ROOT", 0.72f }, { "VOICE_FIFTH", 0.76f }, { "VOICE_OCT", 0.85f }, { "VOICE_NINTH", 0.78f }, { "VOICE_HIGH", 0.68f },
                    { "SYNTH_DETUNE", 0.54f }, { "SYNTH_WIDTH", 0.96f }, { "SYNTH_ATTACK", 0.45f }, { "SYNTH_RELEASE", 2.00f }
                }
            },
            {
                "04 // AURORA STRINGS", "AIRY ENSEMBLE",
                {
                    { "DRONE_OCTAVE", 1.0f },
                    { "OSC_A_TABLE", 4.0f }, { "OSC_A_POS", 0.66f }, { "OSC_A_WARP_MODE", 1.0f }, { "OSC_A_WARP", 0.32f }, { "OSC_A_LEVEL", 0.90f },
                    { "OSC_B_TABLE", 2.0f }, { "OSC_B_POS", 0.60f }, { "OSC_B_OCT", 1.0f },       { "OSC_B_LEVEL", 0.68f }, { "AIR_NOISE", 0.34f },
                    { "FILTER_MODE", 1.0f }, { "MORPH_X", 0.60f },   { "MORPH_Y", 0.72f },       { "SYNTH_RES", 0.24f },   { "SYNTH_DRIVE", 0.22f },
                    { "FILTER_ENV_AMT", 0.36f }, { "LFO1_RATE", 0.44f }, { "LFO1_SHAPE", 0.0f }, { "LFO1_WT_AMT", 0.30f },
                    { "LFO2_RATE", 0.22f }, { "LFO2_SHAPE", 2.0f },  { "LFO2_CUT_AMT", 0.18f },
                    { "SHIMMER_AMT", 0.68f }, { "AMBIENCE_DECAY", 0.89f }, { "SPACE_WARP", 0.80f }, { "SPACE_MOD", 0.60f }, { "SPACE_DAMP", 0.26f }, { "AMBIENCE_MIX", 0.62f },
                    { "VOICE_SUB", 0.24f }, { "VOICE_ROOT", 0.70f }, { "VOICE_FIFTH", 0.74f }, { "VOICE_OCT", 0.84f }, { "VOICE_NINTH", 0.76f }, { "VOICE_HIGH", 0.64f },
                    { "SYNTH_DETUNE", 0.60f }, { "SYNTH_WIDTH", 0.95f }, { "SYNTH_ATTACK", 0.55f }, { "SYNTH_RELEASE", 1.85f }
                }
            },
            {
                "05 // PRISM CHIME PAD", "SPARKLE KEYS",
                {
                    { "DRONE_OCTAVE", 2.0f },
                    { "OSC_A_TABLE", 3.0f }, { "OSC_A_POS", 0.74f }, { "OSC_A_WARP_MODE", 2.0f }, { "OSC_A_WARP", 0.22f }, { "OSC_A_LEVEL", 0.86f },
                    { "OSC_B_TABLE", 2.0f }, { "OSC_B_POS", 0.80f }, { "OSC_B_OCT", 1.0f },       { "OSC_B_LEVEL", 0.82f }, { "AIR_NOISE", 0.30f },
                    { "FILTER_MODE", 1.0f }, { "MORPH_X", 0.70f },   { "MORPH_Y", 0.82f },       { "SYNTH_RES", 0.28f },   { "SYNTH_DRIVE", 0.14f },
                    { "FILTER_ENV_AMT", 0.44f }, { "LFO1_RATE", 0.52f }, { "LFO1_SHAPE", 1.0f }, { "LFO1_WT_AMT", 0.36f },
                    { "LFO2_RATE", 0.26f }, { "LFO2_SHAPE", 0.0f },  { "LFO2_CUT_AMT", 0.22f },
                    { "SHIMMER_AMT", 0.84f }, { "AMBIENCE_DECAY", 0.91f }, { "SPACE_WARP", 0.85f }, { "SPACE_MOD", 0.68f }, { "SPACE_DAMP", 0.18f }, { "AMBIENCE_MIX", 0.64f },
                    { "VOICE_SUB", 0.14f }, { "VOICE_ROOT", 0.62f }, { "VOICE_FIFTH", 0.76f }, { "VOICE_OCT", 0.88f }, { "VOICE_NINTH", 0.84f }, { "VOICE_HIGH", 0.80f },
                    { "SYNTH_DETUNE", 0.50f }, { "SYNTH_WIDTH", 0.96f }, { "SYNTH_ATTACK", 0.18f }, { "SYNTH_RELEASE", 1.75f }
                }
            },
            {
                "06 // WARM SILK BED", "BALANCED PAD",
                {
                    { "DRONE_OCTAVE", 1.0f },
                    { "OSC_A_TABLE", 0.0f }, { "OSC_A_POS", 0.48f }, { "OSC_A_WARP_MODE", 1.0f }, { "OSC_A_WARP", 0.20f }, { "OSC_A_LEVEL", 0.86f },
                    { "OSC_B_TABLE", 1.0f }, { "OSC_B_POS", 0.56f }, { "OSC_B_OCT", 1.0f },       { "OSC_B_LEVEL", 0.68f }, { "AIR_NOISE", 0.24f },
                    { "FILTER_MODE", 0.0f }, { "MORPH_X", 0.52f },   { "MORPH_Y", 0.64f },       { "SYNTH_RES", 0.20f },   { "SYNTH_DRIVE", 0.24f },
                    { "FILTER_ENV_AMT", 0.30f }, { "LFO1_RATE", 0.30f }, { "LFO1_SHAPE", 2.0f }, { "LFO1_WT_AMT", 0.26f },
                    { "LFO2_RATE", 0.16f }, { "LFO2_SHAPE", 0.0f },  { "LFO2_CUT_AMT", 0.18f },
                    { "SHIMMER_AMT", 0.60f }, { "AMBIENCE_DECAY", 0.86f }, { "SPACE_WARP", 0.74f }, { "SPACE_MOD", 0.54f }, { "SPACE_DAMP", 0.32f }, { "AMBIENCE_MIX", 0.58f },
                    { "VOICE_SUB", 0.38f }, { "VOICE_ROOT", 0.76f }, { "VOICE_FIFTH", 0.72f }, { "VOICE_OCT", 0.78f }, { "VOICE_NINTH", 0.68f }, { "VOICE_HIGH", 0.56f },
                    { "SYNTH_DETUNE", 0.44f }, { "SYNTH_WIDTH", 0.90f }, { "SYNTH_ATTACK", 0.32f }, { "SYNTH_RELEASE", 1.50f }
                }
            }
        };
        return presets;
    }

    static void applyPreset (juce::AudioProcessorValueTreeState& apvts, int presetIndex)
    {
        const auto& all = getPresets();
        if (all.empty()) return;
        int idx = ((presetIndex % static_cast<int> (all.size())) + static_cast<int> (all.size())) % static_cast<int> (all.size());
        const auto& preset = all[static_cast<size_t> (idx)];

        for (const auto& kv : preset.values)
        {
            if (auto* param = apvts.getParameter (kv.first))
            {
                float normVal = param->convertTo0to1 (kv.second);
                param->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, normVal));
            }
        }
    }
};
