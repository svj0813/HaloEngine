#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "DSP/PresetBank.h"
#include <array>

// ==============================================================================
// CUSTOM STUDIO DARK / NORDIC HARDWARE LOOKANDFEEL
// ==============================================================================
class HaloLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HaloLookAndFeel()
    {
        setColour (juce::Slider::thumbColourId, juce::Colour (0xff4FD1C5));
        setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff4FD1C5));
        setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff1E2634));
        setColour (juce::TextButton::buttonColourId, juce::Colour (0xff161C27));
        setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff4FD1C5));
        setColour (juce::TextButton::textColourOffId, juce::Colour (0xff94A3B8));
        setColour (juce::TextButton::textColourOnId, juce::Colour (0xff0B0F17));
        setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff151B26));
        setColour (juce::ComboBox::textColourId, juce::Colour (0xffE2E8F0));
        setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff273244));
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff121722));
        setColour (juce::PopupMenu::textColourId, juce::Colour (0xffE2E8F0));
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff4FD1C5));
        setColour (juce::PopupMenu::highlightedTextColourId, juce::Colour (0xff0B0F17));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = juce::jmax (2.2f, radius * 0.11f);
        auto arcRadius = radius - lineW * 0.6f;

        auto accent = slider.findColour (juce::Slider::rotarySliderFillColourId);

        juce::Path backgroundArc;
        backgroundArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(),
                                     arcRadius, arcRadius, 0.0f,
                                     rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour (0xff1C2432));
        g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (slider.isEnabled())
        {
            juce::Path valueArc;
            valueArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(),
                                    arcRadius, arcRadius, 0.0f,
                                    rotaryStartAngle, toAngle, true);
            g.setColour (accent);
            g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        auto knobRadius = arcRadius - lineW * 1.4f;
        if (knobRadius > 4.0f)
        {
            juce::ColourGradient grad (juce::Colour (0xff1E2636), bounds.getCentreX(), bounds.getCentreY() - knobRadius,
                                       juce::Colour (0xff10151F), bounds.getCentreX(), bounds.getCentreY() + knobRadius, false);
            g.setGradientFill (grad);
            g.fillEllipse (bounds.getCentreX() - knobRadius, bounds.getCentreY() - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

            g.setColour (juce::Colour (0xff2E3A4E));
            g.drawEllipse (bounds.getCentreX() - knobRadius, bounds.getCentreY() - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.0f);

            juce::Point<float> thumbPoint (bounds.getCentreX() + (knobRadius * 0.72f) * std::cos (toAngle - juce::MathConstants<float>::halfPi),
                                           bounds.getCentreY() + (knobRadius * 0.72f) * std::sin (toAngle - juce::MathConstants<float>::halfPi));
            g.setColour (accent);
            g.drawLine (bounds.getCentreX(), bounds.getCentreY(), thumbPoint.x, thumbPoint.y, 2.0f);
        }
    }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float, float,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto accent = slider.findColour (juce::Slider::thumbColourId);
        if (style == juce::Slider::LinearVertical)
        {
            float trackW = 6.0f;
            float cx = x + width * 0.5f;
            g.setColour (juce::Colour (0xff171E2B));
            g.fillRoundedRectangle (cx - trackW * 0.5f, static_cast<float> (y), trackW, static_cast<float> (height), 3.0f);

            float fillTop = sliderPos;
            float fillH = (y + height) - fillTop;
            if (fillH > 0.0f)
            {
                g.setColour (accent.withAlpha (0.85f));
                g.fillRoundedRectangle (cx - trackW * 0.5f, fillTop, trackW, fillH, 3.0f);
            }

            g.setColour (juce::Colour (0xffE2E8F0));
            g.fillRoundedRectangle (cx - 10.0f, sliderPos - 4.0f, 20.0f, 8.0f, 3.0f);
            g.setColour (accent);
            g.drawRoundedRectangle (cx - 10.0f, sliderPos - 4.0f, 20.0f, 8.0f, 3.0f, 1.2f);
        }
    }
};

// ==============================================================================
// REAL-TIME SERUM/VITAL WAVETABLE DISPLAY COMPONENT
// ==============================================================================
class WavetablePreviewComponent : public juce::Component
{
public:
    WavetablePreviewComponent (const WavetableBank& bankRef, juce::Colour accentCol)
        : bank (bankRef), accent (accentCol) {}

    void updateState (int newTable, float newPos, float newWarp, int newWarpMode)
    {
        if (tableIdx != newTable || std::abs (wtPos - newPos) > 0.002f
            || std::abs (warpAmt - newWarp) > 0.002f || warpMode != newWarpMode)
        {
            tableIdx = newTable;
            wtPos = newPos;
            warpAmt = newWarp;
            warpMode = newWarpMode;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff0B0F17));
        g.fillRoundedRectangle (b, 6.0f);
        g.setColour (juce::Colour (0xff232D3F));
        g.drawRoundedRectangle (b, 6.0f, 1.0f);

        // Center zero line
        g.setColour (juce::Colour (0xff1A2232));
        g.drawHorizontalLine (static_cast<int> (b.getCentreY()), b.getX() + 4.0f, b.getRight() - 4.0f);

        // Draw subtle background ghost frames (3D wavetable stack look)
        for (int layer = 2; layer >= 1; --layer)
        {
            float ghostPos = juce::jlimit (0.0f, 1.0f, wtPos + layer * 0.08f);
            juce::Path ghostPath;
            const int pts = 64;
            for (int i = 0; i < pts; ++i)
            {
                float normX = static_cast<float> (i) / static_cast<float> (pts - 1);
                float sample = bank.getPreviewSample (tableIdx, ghostPos, normX);
                float px = b.getX() + 6.0f + normX * (b.getWidth() - 12.0f);
                float py = b.getCentreY() - sample * (b.getHeight() * 0.34f) - layer * 3.0f;
                if (i == 0) ghostPath.startNewSubPath (px, py);
                else ghostPath.lineTo (px, py);
            }
            g.setColour (accent.withAlpha (0.14f / static_cast<float> (layer)));
            g.strokePath (ghostPath, juce::PathStrokeType (1.0f));
        }

        // Draw active morphed waveform
        juce::Path wavePath;
        const int steps = 96;
        for (int i = 0; i < steps; ++i)
        {
            double p = static_cast<double> (i) / static_cast<double> (steps - 1);
            double warpedP = p;
            if (warpAmt > 0.001f && warpMode == 1)
            {
                double pivot = 0.5 - warpAmt * 0.42;
                warpedP = (p < pivot) ? (0.5 * (p / pivot)) : (0.5 + 0.5 * ((p - pivot) / (1.0 - pivot)));
            }
            else if (warpAmt > 0.001f && warpMode == 2)
            {
                double synced = p * (1.0 + warpAmt * 2.5);
                warpedP = synced - std::floor (synced);
            }

            float sample = bank.getPreviewSample (tableIdx, wtPos, static_cast<float> (warpedP));
            float px = b.getX() + 6.0f + static_cast<float> (p) * (b.getWidth() - 12.0f);
            float py = b.getCentreY() - sample * (b.getHeight() * 0.38f);
            if (i == 0) wavePath.startNewSubPath (px, py);
            else wavePath.lineTo (px, py);
        }

        g.setColour (accent.withAlpha (0.28f));
        g.strokePath (wavePath, juce::PathStrokeType (4.0f));
        g.setColour (accent);
        g.strokePath (wavePath, juce::PathStrokeType (1.8f));
    }

private:
    const WavetableBank& bank;
    juce::Colour accent;
    int tableIdx = 0;
    float wtPos = 0.5f;
    float warpAmt = 0.0f;
    int warpMode = 0;
};

// ==============================================================================
// INTERACTIVE 2D MORPH PAD (X = TEXTURE / WT SCAN, Y = BRIGHTNESS / CUTOFF)
// ==============================================================================
class MorphPadComponent : public juce::Component
{
public:
    MorphPadComponent (juce::AudioProcessorValueTreeState& state) : apvts (state) {}

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();

        juce::ColourGradient bg (juce::Colour (0xff121824), b.getTopLeft(),
                                 juce::Colour (0xff0B0F17), b.getBottomRight(), false);
        g.setGradientFill (bg);
        g.fillRoundedRectangle (b, 10.0f);

        g.setColour (juce::Colour (0xff232D3F));
        g.drawRoundedRectangle (b, 10.0f, 1.2f);

        g.setColour (juce::Colour (0xff1B2332));
        for (int i = 1; i < 4; ++i)
        {
            float gx = b.getX() + (b.getWidth() * i) / 4.0f;
            float gy = b.getY() + (b.getHeight() * i) / 4.0f;
            g.drawVerticalLine (static_cast<int> (gx), b.getY() + 6.0f, b.getBottom() - 6.0f);
            g.drawHorizontalLine (static_cast<int> (gy), b.getX() + 6.0f, b.getRight() - 6.0f);
        }

        g.setFont (juce::Font (10.5f, juce::Font::bold));
        g.setColour (juce::Colour (0xff64748B));
        g.drawText ("AIRY / CHIME", b.reduced (10.0f), juce::Justification::topRight);
        g.drawText ("WARM / FELT",  b.reduced (10.0f), juce::Justification::bottomLeft);
        g.drawText ("BRIGHT",       b.reduced (10.0f), juce::Justification::topLeft);
        g.drawText ("SHIMMER",      b.reduced (10.0f), juce::Justification::bottomRight);

        float mx = *apvts.getRawParameterValue ("MORPH_X");
        float my = *apvts.getRawParameterValue ("MORPH_Y");

        float px = b.getX() + 14.0f + mx * (b.getWidth() - 28.0f);
        float py = b.getBottom() - 14.0f - my * (b.getHeight() - 28.0f);

        g.setColour (juce::Colour (0xff4FD1C5).withAlpha (0.18f));
        g.fillEllipse (px - 22.0f, py - 22.0f, 44.0f, 44.0f);
        g.setColour (juce::Colour (0xff4FD1C5).withAlpha (0.45f));
        g.drawEllipse (px - 13.0f, py - 13.0f, 26.0f, 26.0f, 1.5f);
        g.setColour (juce::Colour (0xff4FD1C5));
        g.fillEllipse (px - 6.0f, py - 6.0f, 12.0f, 12.0f);
    }

    void mouseDown (const juce::MouseEvent& e) override { updateFromMouse (e); }
    void mouseDrag (const juce::MouseEvent& e) override { updateFromMouse (e); }

private:
    void updateFromMouse (const juce::MouseEvent& e)
    {
        auto b = getLocalBounds().toFloat().reduced (14.0f);
        float nx = juce::jlimit (0.0f, 1.0f, (e.position.x - b.getX()) / b.getWidth());
        float ny = juce::jlimit (0.0f, 1.0f, 1.0f - (e.position.y - b.getY()) / b.getHeight());

        if (auto* px = apvts.getParameter ("MORPH_X"))
            px->setValueNotifyingHost (nx);
        if (auto* py = apvts.getParameter ("MORPH_Y"))
            py->setValueNotifyingHost (ny);

        repaint();
    }

    juce::AudioProcessorValueTreeState& apvts;
};

// ==============================================================================
// MAIN HALO ENGINE EDITOR (STAGE + SERUM/VITAL/SUPERMASSIVE SYNTH WORKSTATION)
// ==============================================================================
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
    void setActiveTab (int tabIndex);
    void loadPresetIndex (int newIndex);

    HaloEngineAudioProcessor& audioProcessor;
    HaloLookAndFeel customLookAndFeel;

    int currentTab = 0;
    int currentPresetIdx = 0;

    // Header Navigation, Preset Browser & Meters
    juce::TextButton stageTabButton { "STAGE" };
    juce::TextButton synthTabButton { "SYNTH ENGINE" };
    juce::TextButton prevPresetButton { "<" };
    juce::TextButton nextPresetButton { ">" };
    juce::ComboBox   presetSelector;
    juce::TextButton keyAssistToggle { "KEY ASSIST: ON" };
    juce::TextButton applyKeyButton  { "APPLY KEY" };
    int detectedKeyCache = -1;

    // ==========================================================================
    // TAB 0: STAGE VIEW COMPONENTS
    // ==========================================================================
    std::array<juce::TextButton, 12> keyButtons;
    juce::TextButton dronePowerButton { "DRONE ACTIVE" };
    juce::TextButton prayerMacroButton { "PRAYER / SWELL" };

    MorphPadComponent morphPad;

    juce::Slider droneLevelSlider, shimmerSlider, decaySlider, autoDuckSlider, mixSlider;
    juce::Slider spaceWarpSlider, spaceModSlider, spaceDampSlider;
    juce::Label  droneLevelLabel, shimmerLabel, decayLabel, autoDuckLabel, mixLabel;
    juce::Label  spaceWarpLabel, spaceModLabel, spaceDampLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> droneLevelAtt, shimmerAtt, decayAtt, autoDuckAtt, mixAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> spaceWarpAtt, spaceModAtt, spaceDampAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> dronePowerAtt, prayerMacroAtt, keyAssistAtt;

    // ==========================================================================
    // TAB 1: SERUM / VITAL / SUPERMASSIVE SYNTH ENGINE COMPONENTS
    // ==========================================================================
    // OSC A & OSC B Wavetable Visualizers + Controls
    WavetablePreviewComponent oscAPreview;
    WavetablePreviewComponent oscBPreview;

    juce::ComboBox oscATableBox, oscAWarpModeBox;
    juce::Slider   oscAPosSlider, oscAWarpSlider, oscALevelSlider;
    juce::Label    oscAPosLabel, oscAWarpLabel, oscALevelLabel;

    juce::ComboBox oscBTableBox;
    juce::Slider   oscBPosSlider, oscBOctSlider, oscBLevelSlider, airNoiseSlider;
    juce::Label    oscBPosLabel, oscBOctLabel, oscBLevelLabel, airNoiseLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oscATableAtt, oscAWarpModeAtt, oscBTableAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   oscAPosAtt, oscAWarpAtt, oscALevelAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   oscBPosAtt, oscBOctAtt, oscBLevelAtt, airNoiseAtt;

    // ZDF Multi-Mode Filter + LFO 1 & LFO 2 + ENV 2
    juce::ComboBox filterModeBox, lfo1ShapeBox, lfo2ShapeBox;
    juce::Slider   filterResSlider, filterEnvSlider, driveSlider;
    juce::Slider   lfo1RateSlider, lfo1WtSlider, lfo2RateSlider, lfo2CutSlider;
    juce::Label    filterResLabel, filterEnvLabel, driveLabel;
    juce::Label    lfo1RateLabel, lfo1WtLabel, lfo2RateLabel, lfo2CutLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterModeAtt, lfo1ShapeAtt, lfo2ShapeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   filterResAtt, filterEnvAtt, driveAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   lfo1RateAtt, lfo1WtAtt, lfo2RateAtt, lfo2CutAtt;

    // 6-Voice Harmonic Drone Mixer Faders + Unison/Glide/ADSR
    std::array<juce::Slider, 6> voiceFaders;
    std::array<juce::Label, 6>  voiceLabels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 6> voiceAtts;

    juce::Slider detuneSlider, widthSlider, glideSlider, attackSlider, releaseSlider;
    juce::Label  detuneLabel, widthLabel, glideLabel, attackLabel, releaseLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detuneAtt, widthAtt, glideAtt, attackAtt, releaseAtt;

    // Playable 2-Octave Keyboard
    juce::MidiKeyboardComponent midiKeyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
