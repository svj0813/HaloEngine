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
                "01 // CATHEDRAL HALO", "WORSHIP PAD",
                {
                    { "OSC_A_TABLE", 0.0f }, { "OSC_A_POS", 0.45f }, { "OSC_A_WARP_MODE", 1.0f }, { "OSC_A_WARP", 0.22f }, { "OSC_A_LEVEL", 0.85f },
                    { "OSC_B_TABLE", 1.0f }, { "OSC_B_POS", 0.52f }, { "OSC_B_OCT", 1.0f },       { "OSC_B_LEVEL", 0.58f }, { "AIR_NOISE", 0.24f },
                    { "FILTER_MODE", 0.0f }, { "MORPH_X", 0.42f },   { "MORPH_Y", 0.52f },       { "SYNTH_RES", 0.24f },   { "SYNTH_DRIVE", 0.25f },
                    { "FILTER_ENV_AMT", 0.35f }, { "LFO1_RATE", 0.35f }, { "LFO1_SHAPE", 2.0f }, { "LFO1_WT_AMT", 0.28f },
                    { "LFO2_RATE", 0.18f }, { "LFO2_SHAPE", 0.0f },  { "LFO2_CUT_AMT", 0.22f },
                    { "SHIMMER_AMT", 0.48f }, { "AMBIENCE_DECAY", 0.86f }, { "SPACE_WARP", 0.74f }, { "SPACE_MOD", 0.55f }, { "SPACE_DAMP", 0.42f }, { "AMBIENCE_MIX", 0.58f },
                    { "VOICE_SUB", 0.80f }, { "VOICE_ROOT", 0.85f }, { "VOICE_FIFTH", 0.52f }, { "VOICE_OCT", 0.48f }, { "VOICE_NINTH", 0.34f }, { "VOICE_HIGH", 0.22f },
                    { "SYNTH_DETUNE", 0.42f }, { "SYNTH_WIDTH", 0.90f }, { "SYNTH_ATTACK", 0.32f }, { "SYNTH_RELEASE", 1.45f }
                }
            },
            {
                "02 // SUPERNOVA CHOIR", "VOCAL CLOUD",
                {
                    { "OSC_A_TABLE", 1.0f }, { "OSC_A_POS", 0.38f }, { "OSC_A_WARP_MODE", 1.0f }, { "OSC_A_WARP", 0.30f }, { "OSC_A_LEVEL", 0.90f },
                    { "OSC_B_TABLE", 4.0f }, { "OSC_B_POS", 0.65f }, { "OSC_B_OCT", 1.0f },       { "OSC_B_LEVEL", 0.68f }, { "AIR_NOISE", 0.36f },
                    { "FILTER_MODE", 1.0f }, { "MORPH_X", 0.55f },   { "MORPH_Y", 0.62f },       { "SYNTH_RES", 0.30f },   { "SYNTH_DRIVE", 0.18f },
                    { "FILTER_ENV_AMT", 0.42f }, { "LFO1_RATE", 0.28f }, { "LFO1_SHAPE", 0.0f }, { "LFO1_WT_AMT", 0.45f },
                    { "LFO2_RATE", 0.14f }, { "LFO2_SHAPE", 2.0f },  { "LFO2_CUT_AMT", 0.28f },
                    { "SHIMMER_AMT", 0.64f }, { "AMBIENCE_DECAY", 0.92f }, { "SPACE_WARP", 0.88f }, { "SPACE_MOD", 0.68f }, { "SPACE_DAMP", 0.34f }, { "AMBIENCE_MIX", 0.68f },
                    { "VOICE_SUB", 0.72f }, { "VOICE_ROOT", 0.88f }, { "VOICE_FIFTH", 0.60f }, { "VOICE_OCT", 0.55f }, { "VOICE_NINTH", 0.46f }, { "VOICE_HIGH", 0.32f },
                    { "SYNTH_DETUNE", 0.54f }, { "SYNTH_WIDTH", 0.96f }, { "SYNTH_ATTACK", 0.55f }, { "SYNTH_RELEASE", 2.10f }
                }
            },
            {
                "03 // FELT PIANO CLOUD", "LABS ORGANIC",
                {
                    { "OSC_A_TABLE", 0.0f }, { "OSC_A_POS", 0.18f }, { "OSC_A_WARP_MODE", 0.0f }, { "OSC_A_WARP", 0.0f },  { "OSC_A_LEVEL", 0.92f },
                    { "OSC_B_TABLE", 2.0f }, { "OSC_B_POS", 0.22f }, { "OSC_B_OCT", 0.0f },       { "OSC_B_LEVEL", 0.35f }, { "AIR_NOISE", 0.48f },
                    { "FILTER_MODE", 0.0f }, { "MORPH_X", 0.25f },   { "MORPH_Y", 0.36f },       { "SYNTH_RES", 0.12f },   { "SYNTH_DRIVE", 0.32f },
                    { "FILTER_ENV_AMT", 0.28f }, { "LFO1_RATE", 0.15f }, { "LFO1_SHAPE", 2.0f }, { "LFO1_WT_AMT", 0.14f },
                    { "LFO2_RATE", 0.10f }, { "LFO2_SHAPE", 0.0f },  { "LFO2_CUT_AMT", 0.12f },
                    { "SHIMMER_AMT", 0.25f }, { "AMBIENCE_DECAY", 0.84f }, { "SPACE_WARP", 0.65f }, { "SPACE_MOD", 0.40f }, { "SPACE_DAMP", 0.68f }, { "AMBIENCE_MIX", 0.54f },
                    { "VOICE_SUB", 0.85f }, { "VOICE_ROOT", 0.90f }, { "VOICE_FIFTH", 0.42f }, { "VOICE_OCT", 0.30f }, { "VOICE_NINTH", 0.20f }, { "VOICE_HIGH", 0.08f },
                    { "SYNTH_DETUNE", 0.26f }, { "SYNTH_WIDTH", 0.78f }, { "SYNTH_ATTACK", 0.08f }, { "SYNTH_RELEASE", 1.65f }
                }
            },
            {
                "04 // GLASS SHIMMER BELLS", "CRYSTAL KEYS",
                {
                    { "OSC_A_TABLE", 2.0f }, { "OSC_A_POS", 0.68f }, { "OSC_A_WARP_MODE", 3.0f }, { "OSC_A_WARP", 0.34f }, { "OSC_A_LEVEL", 0.88f },
                    { "OSC_B_TABLE", 2.0f }, { "OSC_B_POS", 0.82f }, { "OSC_B_OCT", 2.0f },       { "OSC_B_LEVEL", 0.64f }, { "AIR_NOISE", 0.28f },
                    { "FILTER_MODE", 1.0f }, { "MORPH_X", 0.68f },   { "MORPH_Y", 0.76f },       { "SYNTH_RES", 0.26f },   { "SYNTH_DRIVE", 0.14f },
                    { "FILTER_ENV_AMT", 0.48f }, { "LFO1_RATE", 0.65f }, { "LFO1_SHAPE", 1.0f }, { "LFO1_WT_AMT", 0.38f },
                    { "LFO2_RATE", 0.32f }, { "LFO2_SHAPE", 0.0f },  { "LFO2_CUT_AMT", 0.25f },
                    { "SHIMMER_AMT", 0.82f }, { "AMBIENCE_DECAY", 0.94f }, { "SPACE_WARP", 0.90f }, { "SPACE_MOD", 0.72f }, { "SPACE_DAMP", 0.22f }, { "AMBIENCE_MIX", 0.66f },
                    { "VOICE_SUB", 0.55f }, { "VOICE_ROOT", 0.78f }, { "VOICE_FIFTH", 0.58f }, { "VOICE_OCT", 0.66f }, { "VOICE_NINTH", 0.54f }, { "VOICE_HIGH", 0.48f },
                    { "SYNTH_DETUNE", 0.48f }, { "SYNTH_WIDTH", 0.98f }, { "SYNTH_ATTACK", 0.04f }, { "SYNTH_RELEASE", 2.40f }
                }
            },
            {
                "05 // ANAMORPHIC STRINGS", "CINEMATIC SCORE",
                {
                    { "OSC_A_TABLE", 4.0f }, { "OSC_A_POS", 0.58f }, { "OSC_A_WARP_MODE", 1.0f }, { "OSC_A_WARP", 0.38f }, { "OSC_A_LEVEL", 0.90f },
                    { "OSC_B_TABLE", 3.0f }, { "OSC_B_POS", 0.44f }, { "OSC_B_OCT", 0.0f },       { "OSC_B_LEVEL", 0.66f }, { "AIR_NOISE", 0.32f },
                    { "FILTER_MODE", 0.0f }, { "MORPH_X", 0.52f },   { "MORPH_Y", 0.58f },       { "SYNTH_RES", 0.28f },   { "SYNTH_DRIVE", 0.36f },
                    { "FILTER_ENV_AMT", 0.40f }, { "LFO1_RATE", 0.42f }, { "LFO1_SHAPE", 0.0f }, { "LFO1_WT_AMT", 0.32f },
                    { "LFO2_RATE", 0.22f }, { "LFO2_SHAPE", 2.0f },  { "LFO2_CUT_AMT", 0.20f },
                    { "SHIMMER_AMT", 0.44f }, { "AMBIENCE_DECAY", 0.88f }, { "SPACE_WARP", 0.80f }, { "SPACE_MOD", 0.62f }, { "SPACE_DAMP", 0.38f }, { "AMBIENCE_MIX", 0.60f },
                    { "VOICE_SUB", 0.82f }, { "VOICE_ROOT", 0.88f }, { "VOICE_FIFTH", 0.64f }, { "VOICE_OCT", 0.52f }, { "VOICE_NINTH", 0.40f }, { "VOICE_HIGH", 0.26f },
                    { "SYNTH_DETUNE", 0.62f }, { "SYNTH_WIDTH", 0.94f }, { "SYNTH_ATTACK", 0.68f }, { "SYNTH_RELEASE", 1.85f }
                }
            },
            {
                "06 // SOLARIS WARM PAD", "PROPHET ANALOG",
                {
                    { "OSC_A_TABLE", 3.0f }, { "OSC_A_POS", 0.62f }, { "OSC_A_WARP_MODE", 2.0f }, { "OSC_A_WARP", 0.18f }, { "OSC_A_LEVEL", 0.86f },
                    { "OSC_B_TABLE", 0.0f }, { "OSC_B_POS", 0.78f }, { "OSC_B_OCT", -1.0f },      { "OSC_B_LEVEL", 0.72f }, { "AIR_NOISE", 0.16f },
                    { "FILTER_MODE", 0.0f }, { "MORPH_X", 0.48f },   { "MORPH_Y", 0.46f },       { "SYNTH_RES", 0.34f },   { "SYNTH_DRIVE", 0.44f },
                    { "FILTER_ENV_AMT", 0.46f }, { "LFO1_RATE", 0.52f }, { "LFO1_SHAPE", 1.0f }, { "LFO1_WT_AMT", 0.36f },
                    { "LFO2_RATE", 0.26f }, { "LFO2_SHAPE", 0.0f },  { "LFO2_CUT_AMT", 0.24f },
                    { "SHIMMER_AMT", 0.35f }, { "AMBIENCE_DECAY", 0.82f }, { "SPACE_WARP", 0.68f }, { "SPACE_MOD", 0.52f }, { "SPACE_DAMP", 0.48f }, { "AMBIENCE_MIX", 0.52f },
                    { "VOICE_SUB", 0.88f }, { "VOICE_ROOT", 0.86f }, { "VOICE_FIFTH", 0.50f }, { "VOICE_OCT", 0.44f }, { "VOICE_NINTH", 0.30f }, { "VOICE_HIGH", 0.16f },
                    { "SYNTH_DETUNE", 0.56f }, { "SYNTH_WIDTH", 0.88f }, { "SYNTH_ATTACK", 0.28f }, { "SYNTH_RELEASE", 1.35f }
                }
            },
            {
                "07 // DEEP PRAYER SUB", "FOUNDATION BED",
                {
                    { "OSC_A_TABLE", 0.0f }, { "OSC_A_POS", 0.12f }, { "OSC_A_WARP_MODE", 0.0f }, { "OSC_A_WARP", 0.0f },  { "OSC_A_LEVEL", 0.94f },
                    { "OSC_B_TABLE", 1.0f }, { "OSC_B_POS", 0.20f }, { "OSC_B_OCT", 0.0f },       { "OSC_B_LEVEL", 0.42f }, { "AIR_NOISE", 0.14f },
                    { "FILTER_MODE", 0.0f }, { "MORPH_X", 0.22f },   { "MORPH_Y", 0.28f },       { "SYNTH_RES", 0.14f },   { "SYNTH_DRIVE", 0.30f },
                    { "FILTER_ENV_AMT", 0.18f }, { "LFO1_RATE", 0.12f }, { "LFO1_SHAPE", 0.0f }, { "LFO1_WT_AMT", 0.16f },
                    { "LFO2_RATE", 0.09f }, { "LFO2_SHAPE", 2.0f },  { "LFO2_CUT_AMT", 0.14f },
                    { "SHIMMER_AMT", 0.22f }, { "AMBIENCE_DECAY", 0.88f }, { "SPACE_WARP", 0.76f }, { "SPACE_MOD", 0.45f }, { "SPACE_DAMP", 0.72f }, { "AMBIENCE_MIX", 0.56f },
                    { "VOICE_SUB", 0.96f }, { "VOICE_ROOT", 0.92f }, { "VOICE_FIFTH", 0.45f }, { "VOICE_OCT", 0.24f }, { "VOICE_NINTH", 0.12f }, { "VOICE_HIGH", 0.04f },
                    { "SYNTH_DETUNE", 0.30f }, { "SYNTH_WIDTH", 0.82f }, { "SYNTH_ATTACK", 0.45f }, { "SYNTH_RELEASE", 1.90f }
                }
            },
            {
                "08 // ETHERIC FORMANT", "SPECTRAL DRIFT",
                {
                    { "OSC_A_TABLE", 1.0f }, { "OSC_A_POS", 0.72f }, { "OSC_A_WARP_MODE", 3.0f }, { "OSC_A_WARP", 0.28f }, { "OSC_A_LEVEL", 0.84f },
                    { "OSC_B_TABLE", 4.0f }, { "OSC_B_POS", 0.48f }, { "OSC_B_OCT", 1.0f },       { "OSC_B_LEVEL", 0.74f }, { "AIR_NOISE", 0.34f },
                    { "FILTER_MODE", 2.0f }, { "MORPH_X", 0.62f },   { "MORPH_Y", 0.64f },       { "SYNTH_RES", 0.36f },   { "SYNTH_DRIVE", 0.22f },
                    { "FILTER_ENV_AMT", 0.38f }, { "LFO1_RATE", 0.48f }, { "LFO1_SHAPE", 2.0f }, { "LFO1_WT_AMT", 0.52f },
                    { "LFO2_RATE", 0.30f }, { "LFO2_SHAPE", 0.0f },  { "LFO2_CUT_AMT", 0.34f },
                    { "SHIMMER_AMT", 0.70f }, { "AMBIENCE_DECAY", 0.95f }, { "SPACE_WARP", 0.94f }, { "SPACE_MOD", 0.78f }, { "SPACE_DAMP", 0.28f }, { "AMBIENCE_MIX", 0.70f },
                    { "VOICE_SUB", 0.70f }, { "VOICE_ROOT", 0.82f }, { "VOICE_FIFTH", 0.56f }, { "VOICE_OCT", 0.58f }, { "VOICE_NINTH", 0.48f }, { "VOICE_HIGH", 0.36f },
                    { "SYNTH_DETUNE", 0.58f }, { "SYNTH_WIDTH", 0.98f }, { "SYNTH_ATTACK", 0.48f }, { "SYNTH_RELEASE", 2.25f }
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
