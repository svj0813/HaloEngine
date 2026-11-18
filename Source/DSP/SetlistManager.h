#pragma once
#include <JuceHeader.h>
#include "PresetBank.h"
#include <vector>

struct SetlistSlot
{
    juce::String songTitle;
    int presetIndex = 0;
    int keyIndex    = 0;
    int octaveIndex = 1;
};

class HaloSetlistManager
{
public:
    HaloSetlistManager() { loadOrCreateDefault(); }

    static juce::File getSetlistFile()
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("HaloEngine");
        dir.createDirectory();
        return dir.getChildFile ("LiveSetlist.json");
    }

    const std::vector<SetlistSlot>& getSlots() const noexcept { return slots; }
    int getCurrentSlotIndex() const noexcept { return currentSlot; }

    void addSlot (const juce::String& title, int presetIdx, int keyIdx, int octIdx)
    {
        slots.push_back ({ title.trim().isEmpty() ? "SONG " + juce::String (slots.size() + 1) : title.trim(), presetIdx, keyIdx, octIdx });
        currentSlot = static_cast<int> (slots.size()) - 1;
        saveToDisk();
    }

    void removeCurrentSlot()
    {
        if (slots.size() <= 1) return;
        slots.erase (slots.begin() + currentSlot);
        currentSlot = juce::jlimit (0, static_cast<int> (slots.size()) - 1, currentSlot);
        saveToDisk();
    }

    int triggerSlot (int index, juce::AudioProcessorValueTreeState& apvts)
    {
        if (slots.empty()) return 0;
        int count = static_cast<int> (slots.size());
        currentSlot = ((index % count) + count) % count;
        const auto& slot = slots[static_cast<size_t> (currentSlot)];

        HaloPresetBank::applyPreset (apvts, slot.presetIndex);
        if (auto* kParam = apvts.getParameter ("DRONE_KEY"))
            kParam->setValueNotifyingHost (kParam->convertTo0to1 (static_cast<float> (slot.keyIndex)));
        if (auto* oParam = apvts.getParameter ("DRONE_OCTAVE"))
            oParam->setValueNotifyingHost (oParam->convertTo0to1 (static_cast<float> (slot.octaveIndex)));
        return slot.presetIndex;
    }

    void saveToDisk() const
    {
        juce::Array<juce::var> arr;
        for (const auto& s : slots)
        {
            auto* obj = new juce::DynamicObject();
            obj->setProperty ("songTitle",   s.songTitle);
            obj->setProperty ("presetIndex", s.presetIndex);
            obj->setProperty ("keyIndex",    s.keyIndex);
            obj->setProperty ("octaveIndex", s.octaveIndex);
            arr.add (juce::var (obj));
        }
        getSetlistFile().replaceWithText (juce::JSON::toString (juce::var (arr)));
    }

private:
    void loadOrCreateDefault()
    {
        slots.clear();
        auto file = getSetlistFile();
        if (file.existsAsFile())
        {
            auto parsed = juce::JSON::parse (file.loadFileAsString());
            if (auto* arr = parsed.getArray())
            {
                for (const auto& item : *arr)
                {
                    if (auto* obj = item.getDynamicObject())
                    {
                        SetlistSlot s;
                        s.songTitle   = obj->getProperty ("songTitle").toString();
                        s.presetIndex = static_cast<int> (obj->getProperty ("presetIndex"));
                        s.keyIndex    = static_cast<int> (obj->getProperty ("keyIndex"));
                        s.octaveIndex = static_cast<int> (obj->getProperty ("octaveIndex"));
                        slots.push_back (s);
                    }
                }
            }
        }
        if (slots.empty())
        {
            slots = {
                { "01 // OPENING ATMOSPHERE", 0, 0, 1 },
                { "02 // ANTHEM BUILD",       2, 7, 0 },
                { "03 // WORSHIP SWELL",      1, 9, 1 },
                { "04 // ALTAR PRAYER",       5, 5, 0 }
            };
            saveToDisk();
        }
    }

    std::vector<SetlistSlot> slots;
    int currentSlot = 0;
};
