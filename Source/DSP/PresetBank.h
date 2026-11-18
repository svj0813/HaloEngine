#pragma once
#include <JuceHeader.h>
#include <vector>

struct HaloPreset
{
    juce::String name;
    juce::String bank;
    juce::File   file;
    std::vector<std::pair<juce::String, float>> values;
};

class HaloPresetBank
{
public:
    static juce::File getPresetsRootDir()
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                       .getChildFile ("HaloEngine").getChildFile ("Presets");
        dir.getChildFile ("Factory").createDirectory();
        dir.getChildFile ("User").createDirectory();
        return dir;
    }

    static const std::vector<HaloPreset>& getFactoryPresets()
    {
        static const std::vector<HaloPreset> presets = {
            { "01 // CRYSTAL HALO", "FACTORY", juce::File(), {
                { "DRONE_OCTAVE", 1.0f }, { "OSC_A_TABLE", 2.0f }, { "OSC_A_POS", 0.68f }, { "OSC_A_WARP_MODE", 1.0f }, { "OSC_A_WARP", 0.18f }, { "OSC_A_LEVEL", 0.86f },
                { "OSC_B_TABLE", 2.0f }, { "OSC_B_POS", 0.82f }, { "OSC_B_OCT", 1.0f }, { "OSC_B_LEVEL", 0.80f }, { "AIR_NOISE", 0.28f },
                { "FILTER_MODE", 1.0f }, { "MORPH_X", 0.65f }, { "MORPH_Y", 0.74f }, { "SYNTH_RES", 0.24f }, { "SYNTH_DRIVE", 0.12f },
                { "FILTER_ENV_AMT", 0.32f }, { "LFO1_RATE", 0.34f }, { "LFO1_SHAPE", 0.0f }, { "LFO1_WT_AMT", 0.30f },
                { "LFO2_RATE", 0.18f }, { "LFO2_SHAPE", 0.0f }, { "LFO2_CUT_AMT", 0.18f },
                { "SHIMMER_AMT", 0.74f }, { "AMBIENCE_DECAY", 0.90f }, { "SPACE_WARP", 0.82f }, { "SPACE_MOD", 0.60f }, { "SPACE_DAMP", 0.28f }, { "AMBIENCE_MIX", 0.64f },
                { "VOICE_SUB", 0.25f }, { "VOICE_ROOT", 0.72f }, { "VOICE_FIFTH", 0.78f }, { "VOICE_OCT", 0.88f }, { "VOICE_NINTH", 0.82f }, { "VOICE_HIGH", 0.72f },
                { "SYNTH_DETUNE", 0.44f }, { "SYNTH_WIDTH", 0.95f }, { "SYNTH_GLIDE", 0.45f }, { "SYNTH_ATTACK", 0.28f }, { "SYNTH_RELEASE", 1.75f }
            }},
            { "02 // CATHEDRAL CHOIR", "FACTORY", juce::File(), {
                { "DRONE_OCTAVE", 1.0f }, { "OSC_A_TABLE", 1.0f }, { "OSC_A_POS", 0.42f }, { "OSC_A_WARP_MODE", 0.0f }, { "OSC_A_WARP", 0.00f }, { "OSC_A_LEVEL", 0.94f },
                { "OSC_B_TABLE", 1.0f }, { "OSC_B_POS", 0.78f }, { "OSC_B_OCT", 1.0f }, { "OSC_B_LEVEL", 0.74f }, { "AIR_NOISE", 0.42f },
                { "FILTER_MODE", 2.0f }, { "MORPH_X", 0.55f }, { "MORPH_Y", 0.68f }, { "SYNTH_RES", 0.32f }, { "SYNTH_DRIVE", 0.18f },
                { "FILTER_ENV_AMT", 0.38f }, { "LFO1_RATE", 0.24f }, { "LFO1_SHAPE", 0.0f }, { "LFO1_WT_AMT", 0.48f },
                { "LFO2_RATE", 0.14f }, { "LFO2_SHAPE", 2.0f }, { "LFO2_CUT_AMT", 0.24f },
                { "SHIMMER_AMT", 0.66f }, { "AMBIENCE_DECAY", 0.93f }, { "SPACE_WARP", 0.88f }, { "SPACE_MOD", 0.68f }, { "SPACE_DAMP", 0.34f }, { "AMBIENCE_MIX", 0.68f },
                { "VOICE_SUB", 0.35f }, { "VOICE_ROOT", 0.84f }, { "VOICE_FIFTH", 0.80f }, { "VOICE_OCT", 0.86f }, { "VOICE_NINTH", 0.74f }, { "VOICE_HIGH", 0.58f },
                { "SYNTH_DETUNE", 0.56f }, { "SYNTH_WIDTH", 0.96f }, { "SYNTH_GLIDE", 0.65f }, { "SYNTH_ATTACK", 0.55f }, { "SYNTH_RELEASE", 2.10f }
            }},
            { "03 // SUPERSAW HORIZON", "FACTORY", juce::File(), {
                { "DRONE_OCTAVE", 0.0f }, { "OSC_A_TABLE", 3.0f }, { "OSC_A_POS", 0.76f }, { "OSC_A_WARP_MODE", 1.0f }, { "OSC_A_WARP", 0.35f }, { "OSC_A_LEVEL", 0.92f },
                { "OSC_B_TABLE", 0.0f }, { "OSC_B_POS", 0.88f }, { "OSC_B_OCT", 1.0f }, { "OSC_B_LEVEL", 0.76f }, { "AIR_NOISE", 0.22f },
                { "FILTER_MODE", 0.0f }, { "MORPH_X", 0.72f }, { "MORPH_Y", 0.62f }, { "SYNTH_RES", 0.36f }, { "SYNTH_DRIVE", 0.34f },
                { "FILTER_ENV_AMT", 0.45f }, { "LFO1_RATE", 0.48f }, { "LFO1_SHAPE", 1.0f }, { "LFO1_WT_AMT", 0.36f },
                { "LFO2_RATE", 0.22f }, { "LFO2_SHAPE", 0.0f }, { "LFO2_CUT_AMT", 0.28f },
                { "SHIMMER_AMT", 0.58f }, { "AMBIENCE_DECAY", 0.88f }, { "SPACE_WARP", 0.78f }, { "SPACE_MOD", 0.64f }, { "SPACE_DAMP", 0.38f }, { "AMBIENCE_MIX", 0.58f },
                { "VOICE_SUB", 0.52f }, { "VOICE_ROOT", 0.88f }, { "VOICE_FIFTH", 0.82f }, { "VOICE_OCT", 0.84f }, { "VOICE_NINTH", 0.76f }, { "VOICE_HIGH", 0.62f },
                { "SYNTH_DETUNE", 0.72f }, { "SYNTH_WIDTH", 0.98f }, { "SYNTH_GLIDE", 0.35f }, { "SYNTH_ATTACK", 0.38f }, { "SYNTH_RELEASE", 1.80f }
            }},
            { "04 // BOWED CELLO CLOUD", "FACTORY", juce::File(), {
                { "DRONE_OCTAVE", 0.0f }, { "OSC_A_TABLE", 4.0f }, { "OSC_A_POS", 0.54f }, { "OSC_A_WARP_MODE", 1.0f }, { "OSC_A_WARP", 0.28f }, { "OSC_A_LEVEL", 0.92f },
                { "OSC_B_TABLE", 4.0f }, { "OSC_B_POS", 0.82f }, { "OSC_B_OCT", 1.0f }, { "OSC_B_LEVEL", 0.72f }, { "AIR_NOISE", 0.38f },
                { "FILTER_MODE", 1.0f }, { "MORPH_X", 0.50f }, { "MORPH_Y", 0.66f }, { "SYNTH_RES", 0.28f }, { "SYNTH_DRIVE", 0.26f },
                { "FILTER_ENV_AMT", 0.34f }, { "LFO1_RATE", 0.30f }, { "LFO1_SHAPE", 2.0f }, { "LFO1_WT_AMT", 0.38f },
                { "LFO2_RATE", 0.16f }, { "LFO2_SHAPE", 2.0f }, { "LFO2_CUT_AMT", 0.22f },
                { "SHIMMER_AMT", 0.62f }, { "AMBIENCE_DECAY", 0.91f }, { "SPACE_WARP", 0.84f }, { "SPACE_MOD", 0.58f }, { "SPACE_DAMP", 0.32f }, { "AMBIENCE_MIX", 0.62f },
                { "VOICE_SUB", 0.45f }, { "VOICE_ROOT", 0.86f }, { "VOICE_FIFTH", 0.82f }, { "VOICE_OCT", 0.80f }, { "VOICE_NINTH", 0.72f }, { "VOICE_HIGH", 0.55f },
                { "SYNTH_DETUNE", 0.58f }, { "SYNTH_WIDTH", 0.92f }, { "SYNTH_GLIDE", 0.75f }, { "SYNTH_ATTACK", 0.62f }, { "SYNTH_RELEASE", 2.20f }
            }},
            { "05 // CELESTIAL HARP", "FACTORY", juce::File(), {
                { "DRONE_OCTAVE", 2.0f }, { "OSC_A_TABLE", 2.0f }, { "OSC_A_POS", 0.90f }, { "OSC_A_WARP_MODE", 3.0f }, { "OSC_A_WARP", 0.24f }, { "OSC_A_LEVEL", 0.84f },
                { "OSC_B_TABLE", 0.0f }, { "OSC_B_POS", 0.18f }, { "OSC_B_OCT", 0.0f }, { "OSC_B_LEVEL", 0.78f }, { "AIR_NOISE", 0.26f },
                { "FILTER_MODE", 1.0f }, { "MORPH_X", 0.78f }, { "MORPH_Y", 0.78f }, { "SYNTH_RES", 0.26f }, { "SYNTH_DRIVE", 0.10f },
                { "FILTER_ENV_AMT", 0.48f }, { "LFO1_RATE", 0.56f }, { "LFO1_SHAPE", 1.0f }, { "LFO1_WT_AMT", 0.34f },
                { "LFO2_RATE", 0.26f }, { "LFO2_SHAPE", 0.0f }, { "LFO2_CUT_AMT", 0.20f },
                { "SHIMMER_AMT", 0.82f }, { "AMBIENCE_DECAY", 0.92f }, { "SPACE_WARP", 0.86f }, { "SPACE_MOD", 0.66f }, { "SPACE_DAMP", 0.24f }, { "AMBIENCE_MIX", 0.66f },
                { "VOICE_SUB", 0.18f }, { "VOICE_ROOT", 0.68f }, { "VOICE_FIFTH", 0.78f }, { "VOICE_OCT", 0.86f }, { "VOICE_NINTH", 0.84f }, { "VOICE_HIGH", 0.76f },
                { "SYNTH_DETUNE", 0.46f }, { "SYNTH_WIDTH", 0.96f }, { "SYNTH_GLIDE", 0.30f }, { "SYNTH_ATTACK", 0.08f }, { "SYNTH_RELEASE", 2.30f }
            }},
            { "06 // VELVET SANCTUARY", "FACTORY", juce::File(), {
                { "DRONE_OCTAVE", 0.0f }, { "OSC_A_TABLE", 0.0f }, { "OSC_A_POS", 0.36f }, { "OSC_A_WARP_MODE", 0.0f }, { "OSC_A_WARP", 0.00f }, { "OSC_A_LEVEL", 0.92f },
                { "OSC_B_TABLE", 1.0f }, { "OSC_B_POS", 0.32f }, { "OSC_B_OCT", 1.0f }, { "OSC_B_LEVEL", 0.68f }, { "AIR_NOISE", 0.20f },
                { "FILTER_MODE", 0.0f }, { "MORPH_X", 0.42f }, { "MORPH_Y", 0.56f }, { "SYNTH_RES", 0.22f }, { "SYNTH_DRIVE", 0.28f },
                { "FILTER_ENV_AMT", 0.28f }, { "LFO1_RATE", 0.22f }, { "LFO1_SHAPE", 0.0f }, { "LFO1_WT_AMT", 0.24f },
                { "LFO2_RATE", 0.12f }, { "LFO2_SHAPE", 0.0f }, { "LFO2_CUT_AMT", 0.18f },
                { "SHIMMER_AMT", 0.52f }, { "AMBIENCE_DECAY", 0.86f }, { "SPACE_WARP", 0.74f }, { "SPACE_MOD", 0.52f }, { "SPACE_DAMP", 0.42f }, { "AMBIENCE_MIX", 0.56f },
                { "VOICE_SUB", 0.48f }, { "VOICE_ROOT", 0.86f }, { "VOICE_FIFTH", 0.78f }, { "VOICE_OCT", 0.76f }, { "VOICE_NINTH", 0.66f }, { "VOICE_HIGH", 0.50f },
                { "SYNTH_DETUNE", 0.42f }, { "SYNTH_WIDTH", 0.90f }, { "SYNTH_GLIDE", 0.55f }, { "SYNTH_ATTACK", 0.42f }, { "SYNTH_RELEASE", 1.90f }
            }}
        };
        return presets;
    }

    static std::vector<HaloPreset> getAllPresets()
    {
        std::vector<HaloPreset> all = getFactoryPresets();
        auto userDir = getPresetsRootDir().getChildFile ("User");
        auto files = userDir.findChildFiles (juce::File::findFiles, false, "*.halo");
        files.sort();
        for (const auto& f : files)
        {
            HaloPreset p;
            p.name = "USER // " + f.getFileNameWithoutExtension().toUpperCase();
            p.bank = "USER";
            p.file = f;
            all.push_back (p);
        }
        return all;
    }

    static bool saveUserPreset (const juce::String& rawName, juce::AudioProcessorValueTreeState& apvts)
    {
        auto cleanName = rawName.trim().replaceCharacters ("/\\:*?\"<>|", "---------");
        if (cleanName.isEmpty()) return false;
        auto outFile = getPresetsRootDir().getChildFile ("User").getChildFile (cleanName + ".halo");
        auto state = apvts.copyState();
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        if (xml == nullptr) return false;
        xml->setAttribute ("presetName", cleanName);
        return xml->writeTo (outFile);
    }

    static void loadInitSawPatch (juce::AudioProcessorValueTreeState& apvts)
    {
        const std::vector<std::pair<juce::String, float>> initSaw {
            { "DRONE_OCTAVE", 1.0f }, { "OSC_A_TABLE", 0.0f }, { "OSC_A_POS", 1.0f }, { "OSC_A_WARP_MODE", 0.0f }, { "OSC_A_WARP", 0.0f }, { "OSC_A_LEVEL", 0.85f },
            { "OSC_B_TABLE", 0.0f }, { "OSC_B_POS", 0.0f }, { "OSC_B_OCT", 0.0f }, { "OSC_B_LEVEL", 0.0f }, { "AIR_NOISE", 0.0f },
            { "FILTER_MODE", 0.0f }, { "MORPH_X", 0.50f }, { "MORPH_Y", 1.0f }, { "SYNTH_RES", 0.0f }, { "SYNTH_DRIVE", 0.0f },
            { "FILTER_ENV_AMT", 0.0f }, { "LFO1_RATE", 0.25f }, { "LFO1_SHAPE", 0.0f }, { "LFO1_WT_AMT", 0.0f },
            { "LFO2_RATE", 0.25f }, { "LFO2_SHAPE", 0.0f }, { "LFO2_CUT_AMT", 0.0f },
            { "SHIMMER_AMT", 0.0f }, { "AMBIENCE_DECAY", 0.45f }, { "SPACE_WARP", 0.40f }, { "SPACE_MOD", 0.15f }, { "SPACE_DAMP", 0.35f }, { "AMBIENCE_MIX", 0.15f },
            { "VOICE_SUB", 0.0f }, { "VOICE_ROOT", 0.85f }, { "VOICE_FIFTH", 0.0f }, { "VOICE_OCT", 0.0f }, { "VOICE_NINTH", 0.0f }, { "VOICE_HIGH", 0.0f },
            { "SYNTH_DETUNE", 0.0f }, { "SYNTH_WIDTH", 0.50f }, { "SYNTH_GLIDE", 0.15f }, { "SYNTH_ATTACK", 0.02f }, { "SYNTH_RELEASE", 0.35f }
        };
        for (const auto& kv : initSaw)
            if (auto* param = apvts.getParameter (kv.first))
                param->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, param->convertTo0to1 (kv.second)));
    }

    static void applyPreset (juce::AudioProcessorValueTreeState& apvts, int presetIndex)
    {
        const auto all = getAllPresets();
        if (all.empty()) return;
        int idx = ((presetIndex % static_cast<int> (all.size())) + static_cast<int> (all.size())) % static_cast<int> (all.size());
        const auto& preset = all[static_cast<size_t> (idx)];

        if (preset.bank == "USER" && preset.file.existsAsFile())
        {
            if (auto xml = juce::XmlDocument::parse (preset.file))
            {
                if (xml->hasTagName (apvts.state.getType()))
                {
                    apvts.replaceState (juce::ValueTree::fromXml (*xml));
                    return;
                }
            }
        }
        for (const auto& kv : preset.values)
            if (auto* param = apvts.getParameter (kv.first))
                param->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, param->convertTo0to1 (kv.second)));
    }
};
