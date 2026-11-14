#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

// ==============================================================================
// HALO LOOK AND FEEL (MILLED ANODIZED HARDWARE & STUDIO FADERS)
// ==============================================================================
HaloLookAndFeel::HaloLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId,       juce::Colour (0xffe2e8f0));
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
}

juce::Label* HaloLookAndFeel::createSliderTextBox (juce::Slider& slider)
{
    auto* l = LookAndFeel_V4::createSliderTextBox (slider);
    l->setFont (juce::FontOptions (10.0f, juce::Font::bold));
    l->setJustificationType (juce::Justification::centred);
    l->setColour (juce::Label::outlineColourId,    juce::Colours::transparentBlack);
    l->setColour (juce::Label::backgroundColourId, juce::Colour (0xff0b0d12));
    l->setColour (juce::Label::textColourId,       juce::Colour (0xffcbd5e1));
    return l;
}

void HaloLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
    float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();
    float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    auto accent = slider.findColour (juce::Slider::rotarySliderFillColourId);

    const int numTicks = 15;
    float tickRadius = radius - 1.0f;
    for (int i = 0; i < numTicks; ++i)
    {
        float frac = static_cast<float> (i) / static_cast<float> (numTicks - 1);
        float tAngle = rotaryStartAngle + frac * (rotaryEndAngle - rotaryStartAngle);
        bool isLit = (frac <= sliderPos + 0.01f);
        bool isEnd = (i == 0 || i == numTicks - 1 || i == numTicks / 2);

        float dotSize = isEnd ? 2.2f : 1.4f;
        float tx = cx + std::sin (tAngle) * tickRadius;
        float ty = cy - std::cos (tAngle) * tickRadius;

        g.setColour (isLit ? accent : juce::Colour (0xff323846));
        g.fillEllipse (tx - dotSize * 0.5f, ty - dotSize * 0.5f, dotSize, dotSize);
    }

    float arcRadius = radius - 5.5f;
    juce::Path bgArc;
    bgArc.addCentredArc (cx, cy, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (0xff090b0f));
    g.strokePath (bgArc, juce::PathStrokeType (2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (sliderPos > 0.005f)
    {
        juce::Path valArc;
        valArc.addCentredArc (cx, cy, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);
        g.setColour (accent);
        g.strokePath (valArc, juce::PathStrokeType (2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    float capRadius = radius * 0.72f;
    for (int s = 4; s >= 1; --s)
    {
        float spread = static_cast<float> (s) * 1.1f;
        g.setColour (juce::Colours::black.withAlpha (0.16f));
        g.fillEllipse (cx - capRadius + spread * 0.6f, cy - capRadius + spread * 1.1f,
                       capRadius * 2.0f, capRadius * 2.0f);
    }

    juce::ColourGradient skirtGrad (juce::Colour (0xff363c4a), cx - capRadius * 0.6f, cy - capRadius * 0.8f,
                                    juce::Colour (0xff101319), cx + capRadius * 0.6f, cy + capRadius * 0.9f, false);
    g.setGradientFill (skirtGrad);
    g.fillEllipse (cx - capRadius, cy - capRadius, capRadius * 2.0f, capRadius * 2.0f);

    float topRadius = capRadius * 0.84f;
    juce::ColourGradient topGrad (juce::Colour (0xff181b23), cx - topRadius * 0.5f, cy - topRadius * 0.7f,
                                  juce::Colour (0xff242935), cx + topRadius * 0.5f, cy + topRadius * 0.8f, false);
    g.setGradientFill (topGrad);
    g.fillEllipse (cx - topRadius, cy - topRadius, topRadius * 2.0f, topRadius * 2.0f);

    juce::Path pointer;
    float pLen = capRadius * 0.58f;
    float pThick = 2.0f;
    pointer.addRoundedRectangle (-pThick * 0.5f, -capRadius + 2.5f, pThick, pLen, 1.0f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (cx, cy));

    g.setColour (juce::Colours::white.withAlpha (0.92f));
    g.fillPath (pointer);
}

void HaloLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float, float,
                                        const juce::Slider::SliderStyle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    auto accent = slider.findColour (juce::Slider::thumbColourId);

    float trackW = 6.0f;
    auto track = bounds.withSizeKeepingCentre (trackW, bounds.getHeight() - 8.0f);
    g.setColour (juce::Colour (0xff090b10));
    g.fillRoundedRectangle (track, 3.0f);

    auto fillRect = track.withTop (sliderPos);
    g.setColour (accent.withAlpha (0.65f));
    g.fillRoundedRectangle (fillRect, 3.0f);

    float capW = 24.0f;
    float capH = 12.0f;
    juce::Rectangle<float> cap (bounds.getCentreX() - capW * 0.5f, sliderPos - capH * 0.5f, capW, capH);

    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.fillRoundedRectangle (cap.translated (0.0f, 2.0f), 2.5f);

    juce::ColourGradient capGrad (juce::Colour (0xff3a4150), cap.getX(), cap.getY(),
                                  juce::Colour (0xff1a1e26), cap.getX(), cap.getBottom(), false);
    g.setGradientFill (capGrad);
    g.fillRoundedRectangle (cap, 2.5f);

    g.setColour (accent);
    g.fillRect (cap.getX() + 3.0f, cap.getCentreY() - 0.8f, cap.getWidth() - 6.0f, 1.6f);
}

void HaloLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                            const juce::Colour& backgroundColour,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.5f);
    float corner = 4.0f;

    bool isToggled = button.getToggleState();
    auto baseCol = isToggled ? button.findColour (juce::TextButton::buttonOnColourId)
                             : backgroundColour;

    if (shouldDrawButtonAsDown)
        baseCol = baseCol.darker (0.14f);
    else if (shouldDrawButtonAsHighlighted)
        baseCol = baseCol.brighter (0.07f);

    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillRoundedRectangle (bounds.translated (0.0f, 1.5f), corner);

    juce::ColourGradient grad (baseCol.brighter (0.10f), bounds.getX(), bounds.getY(),
                               baseCol.darker (0.18f),   bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (bounds, corner);

    g.setColour (juce::Colours::white.withAlpha (isToggled ? 0.32f : 0.08f));
    g.drawHorizontalLine (static_cast<int> (bounds.getY()), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
}

void HaloLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                      bool, bool)
{
    g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    bool isToggled = button.getToggleState();
    auto textCol = isToggled ? juce::Colours::white
                             : button.findColour (juce::TextButton::textColourOffId);
    g.setColour (textCol);
    g.drawFittedText (button.getButtonText(), button.getLocalBounds().reduced (4, 1),
                      juce::Justification::centred, 1);
}

// ==============================================================================
// TOP PANORAMIC SPECTRUM & HARMONIC CURVE VISUALIZER
// ==============================================================================
PanoramicHarmonicDisplay::PanoramicHarmonicDisplay (HaloEngineAudioProcessor& proc)
    : processor (proc)
{
}

void PanoramicHarmonicDisplay::advanceAnimation (float dt)
{
    animPhase = std::fmod (animPhase + dt * 1.8f, juce::MathConstants<float>::twoPi);
}

void PanoramicHarmonicDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient screenBg (juce::Colour (0xff090b10), bounds.getCentreX(), bounds.getY(),
                                   juce::Colour (0xff0e1118), bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill (screenBg);
    g.fillRoundedRectangle (bounds, 6.0f);

    g.saveState();
    g.reduceClipRegion (bounds.toNearestInt());

    const char* freqLabels[6] = { "50", "120", "400", "1.2k", "4k", "10k" };
    g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
    for (int i = 0; i < 6; ++i)
    {
        float x = bounds.getX() + bounds.getWidth() * (0.10f + i * 0.16f);
        g.setColour (juce::Colours::white.withAlpha (0.035f));
        g.drawVerticalLine (static_cast<int> (x), bounds.getY() + 20.0f, bounds.getBottom() - 4.0f);
        g.setColour (juce::Colour (0xff475569));
        g.drawText (freqLabels[i], static_cast<int> (x - 16), static_cast<int> (bounds.getY() + 6), 32, 12,
                    juce::Justification::centred);
    }

    int   keyIndex   = static_cast<int> (*processor.apvts.getRawParameterValue ("DRONE_KEY"));
    bool  droneOn    = *processor.apvts.getRawParameterValue ("DRONE_ON") > 0.5f;
    float droneLvl   = droneOn ? processor.apvts.getRawParameterValue ("DRONE_LEVEL")->load() : 0.08f;
    float morphX     = *processor.apvts.getRawParameterValue ("MORPH_X");
    float morphY     = *processor.apvts.getRawParameterValue ("MORPH_Y");
    float shimmerAmt = *processor.apvts.getRawParameterValue ("SHIMMER_AMT");
    float decayAmt   = *processor.apvts.getRawParameterValue ("AMBIENCE_DECAY");
    bool  prayerOn   = *processor.apvts.getRawParameterValue ("PRAYER_MACRO") > 0.5f;
    float rms        = (processor.outLevelL.load (std::memory_order_relaxed)
                      + processor.outLevelR.load (std::memory_order_relaxed)) * 0.5f;

    float baseY = bounds.getBottom();
    float h     = bounds.getHeight() - 24.0f;

    g.setColour (juce::Colour (0xff38bdf8).withAlpha (0.14f));
    for (int bar = 0; bar < 96; ++bar)
    {
        float t = static_cast<float> (bar) / 96.0f;
        float bx = bounds.getX() + t * bounds.getWidth();
        float harmonicSpike = std::abs (std::sin (t * 28.0f + keyIndex * 0.52f + animPhase * 0.6f))
                            * std::abs (std::cos (t * 63.0f - animPhase * 1.1f));
        float env = std::exp (-std::pow ((t - (0.35f + morphY * 0.35f)) * 2.4f, 2.0f));
        float barH = (12.0f + rms * 160.0f + shimmerAmt * 38.0f) * harmonicSpike * (0.35f + env);
        g.drawLine (bx, baseY, bx, baseY - juce::jmin (h * 0.85f, barH), 2.0f);
    }

    juce::Path subCurve;
    subCurve.startNewSubPath (bounds.getX(), baseY);
    float subCenter = 0.24f + (keyIndex / 12.0f) * 0.08f;
    float subHeight = h * (0.35f + droneLvl * 0.52f + rms * 0.35f);

    for (float x = bounds.getX(); x <= bounds.getRight(); x += 3.0f)
    {
        float t = (x - bounds.getX()) / bounds.getWidth();
        float bell = std::exp (-std::pow ((t - subCenter) / 0.13f, 2.0f));
        float ripple = 1.0f + 0.04f * std::sin (t * 18.0f + animPhase * 2.0f);
        subCurve.lineTo (x, baseY - subHeight * bell * ripple);
    }
    subCurve.lineTo (bounds.getRight(), baseY);
    subCurve.closeSubPath();

    juce::ColourGradient subGrad (juce::Colour (0xff6366f1).withAlpha (0.72f), bounds.getX() + bounds.getWidth() * 0.15f, baseY - subHeight,
                                  juce::Colour (0xfff97316).withAlpha (0.68f), bounds.getX() + bounds.getWidth() * 0.42f, baseY, false);
    g.setGradientFill (subGrad);
    g.fillPath (subCurve);

    juce::Path shimmerCurve;
    shimmerCurve.startNewSubPath (bounds.getX(), baseY);
    float shimCenter = 0.44f + morphY * 0.22f;
    float shimWidth  = 0.14f + decayAmt * 0.08f;
    float shimHeight = h * (0.28f + shimmerAmt * 0.48f + morphX * 0.22f + rms * 0.30f);

    for (float x = bounds.getX(); x <= bounds.getRight(); x += 3.0f)
    {
        float t = (x - bounds.getX()) / bounds.getWidth();
        float bell = std::exp (-std::pow ((t - shimCenter) / shimWidth, 2.0f));
        float ripple = 1.0f + 0.05f * std::cos (t * 22.0f - animPhase * 2.4f);
        shimmerCurve.lineTo (x, baseY - shimHeight * bell * ripple);
    }
    shimmerCurve.lineTo (bounds.getRight(), baseY);
    shimmerCurve.closeSubPath();

    juce::Colour cStart = prayerOn ? juce::Colour (0xffa855f7) : juce::Colour (0xfffb7185);
    juce::Colour cEnd   = juce::Colour (0xff38bdf8);
    juce::ColourGradient shimGrad (cStart.withAlpha (0.65f), bounds.getX() + bounds.getWidth() * 0.36f, baseY - shimHeight,
                                   cEnd.withAlpha (0.35f),   bounds.getX() + bounds.getWidth() * 0.72f, baseY, false);
    g.setGradientFill (shimGrad);
    g.fillPath (shimmerCurve);

    const juce::StringArray keyNames { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    g.setColour (juce::Colours::white.withAlpha (0.22f));
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("HALO ENGINE  ::  KEY OF " + keyNames[keyIndex],
                bounds.withTrimmedTop (22.0f).withHeight (24.0f), juce::Justification::centred);

    juce::ColourGradient topShadow (juce::Colours::black.withAlpha (0.75f), 0.0f, bounds.getY(),
                                    juce::Colours::transparentBlack,       0.0f, bounds.getY() + 14.0f, false);
    g.setGradientFill (topShadow);
    g.fillRect (bounds.withHeight (14.0f));

    g.restoreState();

    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.drawHorizontalLine (static_cast<int> (bounds.getBottom()), bounds.getX() + 4.0f, bounds.getRight() - 4.0f);
}

// ==============================================================================
// REAL-TIME OSCILLOSCOPE MONITOR (SYNTH ENGINE TAB)
// ==============================================================================
SynthOscilloscope::SynthOscilloscope (HaloEngineAudioProcessor& proc)
    : processor (proc)
{
}

void SynthOscilloscope::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour (juce::Colour (0xff090b10));
    g.fillRoundedRectangle (bounds, 6.0f);

    g.setColour (juce::Colours::white.withAlpha (0.04f));
    g.drawHorizontalLine (static_cast<int> (bounds.getCentreY()), bounds.getX() + 6.0f, bounds.getRight() - 6.0f);
    for (int i = 1; i < 6; ++i)
    {
        float gx = bounds.getX() + (bounds.getWidth() * i) / 6.0f;
        g.drawVerticalLine (static_cast<int> (gx), bounds.getY() + 6.0f, bounds.getBottom() - 6.0f);
    }

    juce::Path wave;
    float midY = bounds.getCentreY();
    float ampH = bounds.getHeight() * 0.42f;

    for (int i = 0; i < HaloEngineAudioProcessor::scopeSize; ++i)
    {
        float t = static_cast<float> (i) / static_cast<float> (HaloEngineAudioProcessor::scopeSize - 1);
        float wx = bounds.getX() + 6.0f + t * (bounds.getWidth() - 12.0f);
        float sample = processor.scopeSamples[static_cast<size_t> (i)].load (std::memory_order_relaxed);
        float wy = midY - sample * ampH;

        if (i == 0)
            wave.startNewSubPath (wx, wy);
        else
            wave.lineTo (wx, wy);
    }

    g.setColour (juce::Colour (0xff38bdf8).withAlpha (0.25f));
    g.strokePath (wave, juce::PathStrokeType (5.0f));
    g.setColour (juce::Colour (0xff38bdf8));
    g.strokePath (wave, juce::PathStrokeType (1.8f));

    g.setFont (juce::FontOptions (9.0f, juce::Font::bold));
    g.setColour (juce::Colour (0xff64748b));
    g.drawText ("HARMONIC WAVEFORM MONITOR", bounds.reduced (10.0f, 6.0f), juce::Justification::topLeft);
}

// ==============================================================================
// VERTICAL CALIBRATED LED METER STRIP
// ==============================================================================
VerticalMeterStrip::VerticalMeterStrip (const juce::String& labelText, bool isGainReduction)
    : title (labelText), duckMode (isGainReduction)
{
}

void VerticalMeterStrip::setLevels (float newL, float newR)
{
    levelL += (juce::jlimit (0.0f, 1.0f, newL) - levelL) * 0.30f;
    levelR += (juce::jlimit (0.0f, 1.0f, newR) - levelR) * 0.30f;
}

void VerticalMeterStrip::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour (juce::Colour (0xff0b0d12));
    g.fillRoundedRectangle (bounds, 4.0f);

    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.drawHorizontalLine (static_cast<int> (bounds.getBottom()), bounds.getX() + 2.0f, bounds.getRight() - 2.0f);

    auto labelArea = bounds.removeFromBottom (18.0f);
    g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
    g.setColour (juce::Colour (0xff64748b));
    g.drawText (title, labelArea, juce::Justification::centred);

    auto trackArea = bounds.reduced (6.0f, 8.0f);
    int segments = 22;
    float gap = 2.0f;
    float segH = (trackArea.getHeight() - gap * (segments - 1)) / static_cast<float> (segments);

    auto drawColumn = [&] (juce::Rectangle<float> col, float val) {
        for (int i = 0; i < segments; ++i)
        {
            float norm = static_cast<float> (i) / static_cast<float> (segments - 1);
            float y = col.getBottom() - (i + 1) * segH - i * gap;
            juce::Rectangle<float> seg (col.getX(), y, col.getWidth(), segH);

            bool lit = (norm <= val && val > 0.01f);
            juce::Colour segCol;
            if (duckMode)
                segCol = juce::Colour (0xfff59e0b);
            else if (norm > 0.85f)
                segCol = juce::Colour (0xfff43f5e);
            else if (norm > 0.65f)
                segCol = juce::Colour (0xfffacc15);
            else
                segCol = juce::Colour (0xff38bdf8);

            g.setColour (lit ? segCol : juce::Colour (0xff151922));
            g.fillRect (seg);
        }
    };

    if (duckMode)
    {
        drawColumn (trackArea.reduced (1.5f, 0.0f), levelL);
    }
    else
    {
        float halfW = (trackArea.getWidth() - 2.0f) * 0.5f;
        drawColumn (trackArea.withWidth (halfW), levelL);
        drawColumn (trackArea.withTrimmedLeft (halfW + 2.0f), levelR);
    }
}

// ==============================================================================
// ANODIZED 1-OCTAVE HARDWARE PIANO KEYBED
// ==============================================================================
HardwareKeybed::HardwareKeybed (HaloEngineAudioProcessor& proc)
    : processor (proc)
{
    setRepaintsOnMouseActivity (true);
}

juce::Rectangle<float> HardwareKeybed::getNaturalRect (int whiteIdx) const
{
    auto b = getLocalBounds().toFloat().reduced (3.0f, 3.0f);
    float gap = 3.0f;
    float w = (b.getWidth() - gap * 6.0f) / 7.0f;
    return { b.getX() + whiteIdx * (w + gap), b.getY(), w, b.getHeight() };
}

juce::Rectangle<float> HardwareKeybed::getSharpRect (int sharpSlot) const
{
    static constexpr int leftWhite[5] = { 0, 1, 3, 4, 5 };
    auto wLeft  = getNaturalRect (leftWhite[sharpSlot]);
    auto wRight = getNaturalRect (leftWhite[sharpSlot] + 1);

    float centerX = (wLeft.getRight() + wRight.getX()) * 0.5f;
    float sharpW  = wLeft.getWidth() * 0.72f;
    float sharpH  = wLeft.getHeight() * 0.56f;
    return { centerX - sharpW * 0.5f, wLeft.getY() - 1.0f, sharpW, sharpH };
}

int HardwareKeybed::hitTestNote (juce::Point<float> pos) const
{
    static constexpr int sharpNotes[5]   = { 1, 3, 6, 8, 10 };
    static constexpr int naturalNotes[7] = { 0, 2, 4, 5, 7, 9, 11 };

    for (int s = 0; s < 5; ++s)
        if (getSharpRect (s).expanded (2.0f).contains (pos))
            return sharpNotes[s];

    for (int w = 0; w < 7; ++w)
        if (getNaturalRect (w).contains (pos))
            return naturalNotes[w];

    return -1;
}

void HardwareKeybed::mouseDown (const juce::MouseEvent& e)
{
    int note = hitTestNote (e.position);
    if (note >= 0)
    {
        if (auto* keyParam = processor.apvts.getParameter ("DRONE_KEY"))
        {
            keyParam->beginChangeGesture();
            keyParam->setValueNotifyingHost (keyParam->convertTo0to1 (static_cast<float> (note)));
            keyParam->endChangeGesture();
        }
        repaint();
    }
}

void HardwareKeybed::mouseDrag (const juce::MouseEvent& e) { mouseDown (e); }

void HardwareKeybed::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff0a0c10));
    g.fillRoundedRectangle (bounds, 5.0f);

    int activeKey = static_cast<int> (*processor.apvts.getRawParameterValue ("DRONE_KEY"));
    static constexpr int naturalNotes[7] = { 0, 2, 4, 5, 7, 9, 11 };
    static constexpr int sharpNotes[5]   = { 1, 3, 6, 8, 10 };

    for (int w = 0; w < 7; ++w)
    {
        int note = naturalNotes[w];
        bool isActive = (note == activeKey);
        auto r = getNaturalRect (w);

        g.setColour (juce::Colours::black.withAlpha (0.65f));
        g.fillRoundedRectangle (r.translated (0.0f, 1.5f), 3.5f);

        juce::Colour topCol = isActive ? juce::Colour (0xff2c3547) : juce::Colour (0xff232834);
        juce::Colour botCol = isActive ? juce::Colour (0xff1d2432) : juce::Colour (0xff171a22);
        juce::ColourGradient keyGrad (topCol, r.getX(), r.getY(), botCol, r.getX(), r.getBottom(), false);
        g.setGradientFill (keyGrad);
        g.fillRoundedRectangle (r, 3.5f);

        g.setColour (juce::Colours::white.withAlpha (isActive ? 0.24f : 0.08f));
        g.drawHorizontalLine (static_cast<int> (r.getY()), r.getX() + 2.0f, r.getRight() - 2.0f);

        if (isActive)
        {
            auto ledBar = r.withTop (r.getBottom() - 4.5f).reduced (4.0f, 0.5f);
            g.setColour (juce::Colour (0xff38bdf8).withAlpha (0.35f));
            g.fillRoundedRectangle (ledBar.expanded (2.0f, 1.5f), 2.0f);
            g.setColour (juce::Colour (0xff38bdf8));
            g.fillRoundedRectangle (ledBar, 1.5f);
        }

        g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
        g.setColour (isActive ? juce::Colours::white : juce::Colour (0xff94a3b8));
        g.drawText (noteNames[note], r.withTop (r.getBottom() - 26.0f).withHeight (18.0f),
                    juce::Justification::centred);
    }

    for (int s = 0; s < 5; ++s)
    {
        int note = sharpNotes[s];
        bool isActive = (note == activeKey);
        auto r = getSharpRect (s);

        g.setColour (juce::Colours::black.withAlpha (0.85f));
        g.fillRoundedRectangle (r.translated (1.0f, 2.5f), 3.0f);

        juce::Colour topCol = isActive ? juce::Colour (0xff1a2230) : juce::Colour (0xff11141b);
        juce::Colour botCol = isActive ? juce::Colour (0xff101622) : juce::Colour (0xff090b0f);
        juce::ColourGradient sharpGrad (topCol, r.getX(), r.getY(), botCol, r.getX(), r.getBottom(), false);
        g.setGradientFill (sharpGrad);
        g.fillRoundedRectangle (r, 3.0f);

        g.setColour (juce::Colours::white.withAlpha (isActive ? 0.28f : 0.11f));
        g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 0.8f);

        if (isActive)
        {
            auto ledBar = r.withTop (r.getBottom() - 4.0f).reduced (3.5f, 0.5f);
            g.setColour (juce::Colour (0xff38bdf8).withAlpha (0.4f));
            g.fillRoundedRectangle (ledBar.expanded (1.5f), 1.5f);
            g.setColour (juce::Colour (0xff38bdf8));
            g.fillRoundedRectangle (ledBar, 1.0f);
        }

        g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
        g.setColour (isActive ? juce::Colour (0xff38bdf8) : juce::Colour (0xff64748b));
        g.drawText (noteNames[note], r.withTop (r.getBottom() - 21.0f).withHeight (15.0f),
                    juce::Justification::centred);
    }
}

// ==============================================================================
// MILLED HARDWARE TOGGLE SWITCH & OLED PITCH READOUT
// ==============================================================================
HardwareDroneSwitch::HardwareDroneSwitch (HaloEngineAudioProcessor& proc)
    : processor (proc)
{
    setRepaintsOnMouseActivity (true);
}

void HardwareDroneSwitch::mouseDown (const juce::MouseEvent&)
{
    if (auto* param = processor.apvts.getParameter ("DRONE_ON"))
    {
        bool current = param->getValue() > 0.5f;
        param->beginChangeGesture();
        param->setValueNotifyingHost (current ? 0.0f : 1.0f);
        param->endChangeGesture();
        repaint();
    }
}

void HardwareDroneSwitch::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    bool isOn = *processor.apvts.getRawParameterValue ("DRONE_ON") > 0.5f;
    int keyIdx = static_cast<int> (*processor.apvts.getRawParameterValue ("DRONE_KEY"));

    auto oledBox = bounds.removeFromTop (24.0f);
    g.setColour (juce::Colour (0xff090b10));
    g.fillRoundedRectangle (oledBox, 4.0f);

    double hz = 440.0 * std::pow (2.0, ((36.0 + keyIdx) - 69.0) / 12.0);
    juce::String readout = noteNames[keyIdx] + "2 // " + juce::String (static_cast<int> (std::round (hz))) + "Hz";

    g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
    g.setColour (isOn ? juce::Colour (0xfff97316) : juce::Colour (0xff475569));
    g.drawText (readout, oledBox, juce::Justification::centred);

    bounds.removeFromTop (6.0f);

    auto switchHousing = bounds.reduced (10.0f, 2.0f);
    g.setColour (juce::Colour (0xff090b0f));
    g.fillRoundedRectangle (switchHousing, 5.0f);
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.drawHorizontalLine (static_cast<int> (switchHousing.getBottom()),
                          switchHousing.getX() + 2.0f, switchHousing.getRight() - 2.0f);

    auto track = switchHousing.withSizeKeepingCentre (18.0f, switchHousing.getHeight() - 22.0f);
    g.setColour (juce::Colour (0xff050608));
    g.fillRoundedRectangle (track, 4.0f);

    float ledY = switchHousing.getY() + 7.0f;
    float cx   = switchHousing.getCentreX();
    if (isOn)
    {
        g.setColour (juce::Colour (0xfff97316).withAlpha (0.4f));
        g.fillEllipse (cx - 5.5f, ledY - 5.5f, 11.0f, 11.0f);
        g.setColour (juce::Colour (0xfff97316));
    }
    else
    {
        g.setColour (juce::Colour (0xff272c38));
    }
    g.fillEllipse (cx - 2.5f, ledY - 2.5f, 5.0f, 5.0f);

    float batH = track.getHeight() * 0.52f;
    auto batRect = isOn ? track.withHeight (batH)
                        : track.withTop (track.getBottom() - batH);

    g.setColour (juce::Colours::black.withAlpha (0.75f));
    g.fillRoundedRectangle (batRect.translated (0.0f, 2.0f), 3.5f);

    juce::ColourGradient batGrad (isOn ? juce::Colour (0xfff97316) : juce::Colour (0xff3a4150),
                                  batRect.getX(), batRect.getY(),
                                  isOn ? juce::Colour (0xffc2410c) : juce::Colour (0xff1c2029),
                                  batRect.getX(), batRect.getBottom(), false);
    g.setGradientFill (batGrad);
    g.fillRoundedRectangle (batRect, 3.5f);

    g.setColour (juce::Colours::white.withAlpha (isOn ? 0.45f : 0.18f));
    g.drawHorizontalLine (static_cast<int> (batRect.getY() + 1.0f),
                          batRect.getX() + 2.0f, batRect.getRight() - 2.0f);

    g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
    g.setColour (isOn ? juce::Colour (0xffe2e8f0) : juce::Colour (0xff64748b));
    g.drawText (isOn ? "ACTIVE" : "MUTED",
                switchHousing.withTop (switchHousing.getBottom() - 13.0f).withHeight (11.0f),
                juce::Justification::centred);
}

// ==============================================================================
// COMPACT RECESSED OLED X/Y MORPH PAD
// ==============================================================================
HarmonicXYPad::HarmonicXYPad (HaloEngineAudioProcessor& proc)
    : processor (proc)
{
    setRepaintsOnMouseActivity (true);
}

void HarmonicXYPad::advanceAnimation (float dt)
{
    animPhase = std::fmod (animPhase + dt * 1.5f, juce::MathConstants<float>::twoPi);
}

void HarmonicXYPad::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour (juce::Colour (0xff0a0c11));
    g.fillRoundedRectangle (bounds, 5.0f);

    g.setColour (juce::Colours::white.withAlpha (0.035f));
    for (int i = 1; i < 4; ++i)
    {
        float gx = bounds.getX() + (bounds.getWidth() * i) / 4.0f;
        float gy = bounds.getY() + (bounds.getHeight() * i) / 4.0f;
        g.drawLine (gx, bounds.getY() + 4.0f, gx, bounds.getBottom() - 4.0f, 1.0f);
        g.drawLine (bounds.getX() + 4.0f, gy, bounds.getRight() - 4.0f, gy, 1.0f);
    }

    float normX = *processor.apvts.getRawParameterValue ("MORPH_X");
    float normY = *processor.apvts.getRawParameterValue ("MORPH_Y");
    bool prayerOn = *processor.apvts.getRawParameterValue ("PRAYER_MACRO") > 0.5f;

    float px = bounds.getX() + normX * bounds.getWidth();
    float py = bounds.getBottom() - normY * bounds.getHeight();

    juce::Colour activeCol = juce::Colour (0xfff97316).interpolatedWith (juce::Colour (0xff38bdf8), (normX + normY) * 0.5f);
    if (prayerOn)
        activeCol = juce::Colour (0xffa855f7);

    g.saveState();
    g.reduceClipRegion (bounds.toNearestInt());

    float glowRad = 44.0f + normY * 22.0f;
    juce::ColourGradient glow (activeCol.withAlpha (0.42f), px, py,
                               activeCol.withAlpha (0.0f),  px + glowRad, py + glowRad, true);
    g.setGradientFill (glow);
    g.fillEllipse (px - glowRad, py - glowRad, glowRad * 2.0f, glowRad * 2.0f);

    g.setColour (activeCol.withAlpha (0.28f));
    g.drawLine (px, bounds.getY(), px, bounds.getBottom(), 1.0f);
    g.drawLine (bounds.getX(), py, bounds.getRight(), py, 1.0f);

    g.setColour (activeCol);
    g.fillEllipse (px - 7.0f, py - 7.0f, 14.0f, 14.0f);
    g.setColour (juce::Colour (0xff0a0c11));
    g.fillEllipse (px - 4.0f, py - 4.0f, 8.0f, 8.0f);
    g.setColour (juce::Colours::white);
    g.fillEllipse (px - 1.8f, py - 1.8f, 3.6f, 3.6f);

    juce::ColourGradient topShadow (juce::Colours::black.withAlpha (0.65f), 0.0f, bounds.getY(),
                                    juce::Colours::transparentBlack,       0.0f, bounds.getY() + 8.0f, false);
    g.setGradientFill (topShadow);
    g.fillRect (bounds.withHeight (8.0f));

    g.restoreState();

    g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
    g.setColour (juce::Colour (0xff64748b));
    g.drawText ("WARM",   bounds.reduced (7.0f), juce::Justification::bottomLeft);
    g.drawText ("CHOIR",  bounds.reduced (7.0f), juce::Justification::bottomRight);
    g.drawText ("DARK",   bounds.reduced (7.0f), juce::Justification::topLeft);
    g.drawText ("BRIGHT", bounds.reduced (7.0f), juce::Justification::topRight);

    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.drawHorizontalLine (static_cast<int> (bounds.getBottom()), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
}

void HarmonicXYPad::mouseDown (const juce::MouseEvent& e) { updateFromPosition (e.position); }
void HarmonicXYPad::mouseDrag (const juce::MouseEvent& e) { updateFromPosition (e.position); }

void HarmonicXYPad::updateFromPosition (juce::Point<float> pos)
{
    auto bounds = getLocalBounds().toFloat();
    float normX = juce::jlimit (0.0f, 1.0f, (pos.x - bounds.getX()) / bounds.getWidth());
    float normY = juce::jlimit (0.0f, 1.0f, 1.0f - ((pos.y - bounds.getY()) / bounds.getHeight()));

    if (auto* paramX = processor.apvts.getParameter ("MORPH_X"))
    {
        paramX->beginChangeGesture();
        paramX->setValueNotifyingHost (normX);
        paramX->endChangeGesture();
    }
    if (auto* paramY = processor.apvts.getParameter ("MORPH_Y"))
    {
        paramY->beginChangeGesture();
        paramY->setValueNotifyingHost (normY);
        paramY->endChangeGesture();
    }
    repaint();
}

// ==============================================================================
// MAIN DUAL-TAB FLAGSHIP VST EDITOR
// ==============================================================================
PluginEditor::PluginEditor (HaloEngineAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      canopyDisplay (p), keybed (p), droneSwitch (p), xyPad (p), oscilloscope (p),
      synthKeyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&haloLookAndFeel);

    // --- TOP TAB SWITCHER (STAGE VIEW vs SYNTH ENGINE) ---
    tabMainBtn.setClickingTogglesState (true);
    tabMainBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff14171f));
    tabMainBtn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff0284c7));
    tabMainBtn.onClick = [this] { switchTab (0); };
    addAndMakeVisible (tabMainBtn);

    tabSynthBtn.setClickingTogglesState (true);
    tabSynthBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff14171f));
    tabSynthBtn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff7c3aed));
    tabSynthBtn.onClick = [this] { switchTab (1); };
    addAndMakeVisible (tabSynthBtn);

    // --- TOP PRESET TOOLBAR ---
    prevPresetBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff14171f));
    prevPresetBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff94a3b8));
    prevPresetBtn.onClick = [this] { applyPreset ((currentPresetIdx + 3) % 4); };
    addAndMakeVisible (prevPresetBtn);

    presetNameBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff10131a));
    presetNameBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffe2e8f0));
    presetNameBtn.onClick = [this] { applyPreset ((currentPresetIdx + 1) % 4); };
    addAndMakeVisible (presetNameBtn);

    nextPresetBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff14171f));
    nextPresetBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff94a3b8));
    nextPresetBtn.onClick = [this] { applyPreset ((currentPresetIdx + 1) % 4); };
    addAndMakeVisible (nextPresetBtn);

    // --- FLANKING METERS ---
    addAndMakeVisible (leftDuckMeter);
    addAndMakeVisible (rightOutMeter);

    // ==========================================================================
    // TAB 0: STAGE PERFORMANCE COMPONENTS
    // ==========================================================================
    addAndMakeVisible (canopyDisplay);
    addAndMakeVisible (keybed);
    addAndMakeVisible (droneSwitch);

    suggestKeyBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff12151d));
    suggestKeyBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff94a3b8));
    suggestKeyBtn.onClick = [this] {
        if (currentSuggestedKey >= 0 && currentSuggestedKey < 12)
        {
            if (auto* keyParam = audioProcessor.apvts.getParameter ("DRONE_KEY"))
            {
                keyParam->beginChangeGesture();
                keyParam->setValueNotifyingHost (keyParam->convertTo0to1 (static_cast<float> (currentSuggestedKey)));
                keyParam->endChangeGesture();
            }
        }
    };
    addAndMakeVisible (suggestKeyBtn);

    keyAssistToggle.setClickingTogglesState (true);
    keyAssistToggle.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff12151d));
    keyAssistToggle.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff2a3242));
    addAndMakeVisible (keyAssistToggle);
    keyAssistAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        audioProcessor.apvts, "KEY_DETECT_ON", keyAssistToggle);

    configureKnob (droneLevelSlider, droneLevelLabel, "DRONE", juce::Colour (0xfff97316));
    droneLevelAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "DRONE_LEVEL", droneLevelSlider);

    addAndMakeVisible (xyPad);

    configureKnob (shimmerSlider, shimmerLabel, "SHIMMER", juce::Colour (0xff38bdf8));
    shimmerAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "SHIMMER_AMT", shimmerSlider);

    configureKnob (decaySlider, decayLabel, "SPACE", juce::Colour (0xff818cf8));
    decayAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "AMBIENCE_DECAY", decaySlider);

    configureKnob (duckSlider, duckLabel, "DUCK", juce::Colour (0xfff59e0b));
    duckAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "AUTO_DUCK", duckSlider);

    configureKnob (mixSlider, mixLabel, "MIX", juce::Colour (0xff38bdf8));
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "AMBIENCE_MIX", mixSlider);

    prayerMacroBtn.setClickingTogglesState (true);
    prayerMacroBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff1b1629));
    prayerMacroBtn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff7c3aed));
    addAndMakeVisible (prayerMacroBtn);
    prayerMacroAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        audioProcessor.apvts, "PRAYER_MACRO", prayerMacroBtn);

    // ==========================================================================
    // TAB 1: SYNTH & VOICING ENGINE LAB COMPONENTS
    // ==========================================================================
    addChildComponent (oscilloscope);

    const char* voiceParamIds[6] = { "VOICE_SUB", "VOICE_ROOT", "VOICE_FIFTH", "VOICE_OCT", "VOICE_NINTH", "VOICE_HIGH" };
    const char* voiceNames[6]    = { "SUB -1", "ROOT", "FIFTH", "OCT +1", "NINTH", "HI 5TH" };
    const juce::Colour voiceCols[6] = {
        juce::Colour (0xfff97316), juce::Colour (0xff38bdf8), juce::Colour (0xff818cf8),
        juce::Colour (0xff38bdf8), juce::Colour (0xffa855f7), juce::Colour (0xfffb7185)
    };

    for (int i = 0; i < 6; ++i)
    {
        configureVerticalFader (voiceFaders[static_cast<size_t> (i)],
                                voiceLabels[static_cast<size_t> (i)],
                                voiceNames[i], voiceCols[i]);
        voiceAttachments[static_cast<size_t> (i)] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            audioProcessor.apvts, voiceParamIds[i], voiceFaders[static_cast<size_t> (i)]);
    }

    auto initSynthKnob = [&] (juce::Slider& s, juce::Label& l, const char* paramId, const char* name, juce::Colour col,
                              std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& att)
    {
        configureKnob (s, l, name, col);
        s.setVisible (false);
        l.setVisible (false);
        att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, paramId, s);
    };

    initSynthKnob (detuneSlider,  detuneLabel,  "SYNTH_DETUNE",  "DRIFT",   juce::Colour (0xfff97316), detuneAttach);
    initSynthKnob (widthSlider,   widthLabel,   "SYNTH_WIDTH",   "WIDTH",   juce::Colour (0xff38bdf8), widthAttach);
    initSynthKnob (resSlider,     resLabel,     "SYNTH_RES",     "RES",     juce::Colour (0xfffb7185), resAttach);
    initSynthKnob (driveSlider,   driveLabel,   "SYNTH_DRIVE",   "DRIVE",   juce::Colour (0xfff59e0b), driveAttach);
    initSynthKnob (glideSlider,   glideLabel,   "SYNTH_GLIDE",   "GLIDE",   juce::Colour (0xffa855f7), glideAttach);
    initSynthKnob (attackSlider,  attackLabel,  "SYNTH_ATTACK",  "ATTACK",  juce::Colour (0xff10b981), attackAttach);
    initSynthKnob (releaseSlider, releaseLabel, "SYNTH_RELEASE", "RELEASE", juce::Colour (0xff818cf8), releaseAttach);

    synthKeyboard.setAvailableRange (36, 72);
    synthKeyboard.setKeyWidth (26.0f);
    synthKeyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId,         juce::Colour (0xff232834));
    synthKeyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId,         juce::Colour (0xff0c0e14));
    synthKeyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId,  juce::Colour (0xff12151c));
    synthKeyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId,    juce::Colour (0xff38bdf8));
    synthKeyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, juce::Colour (0xff38bdf8).withAlpha (0.35f));
    synthKeyboard.setOctaveForMiddleC (4);
    addChildComponent (synthKeyboard);

    switchTab (0);
    setSize (960, 560);
    setResizable (true, true);
    startTimerHz (30);
}

PluginEditor::~PluginEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void PluginEditor::switchTab (int tabIndex)
{
    activeTab = tabIndex;
    bool isStage = (activeTab == 0);
    bool isSynth = (activeTab == 1);

    tabMainBtn.setToggleState  (isStage, juce::dontSendNotification);
    tabSynthBtn.setToggleState (isSynth, juce::dontSendNotification);

    canopyDisplay.setVisible (isStage);
    keybed.setVisible (isStage);
    droneSwitch.setVisible (isStage);
    suggestKeyBtn.setVisible (isStage);
    keyAssistToggle.setVisible (isStage);
    droneLevelSlider.setVisible (isStage);
    droneLevelLabel.setVisible (isStage);
    xyPad.setVisible (isStage);
    shimmerSlider.setVisible (isStage); shimmerLabel.setVisible (isStage);
    decaySlider.setVisible (isStage);   decayLabel.setVisible (isStage);
    duckSlider.setVisible (isStage);    duckLabel.setVisible (isStage);
    mixSlider.setVisible (isStage);     mixLabel.setVisible (isStage);
    prayerMacroBtn.setVisible (isStage);

    oscilloscope.setVisible (isSynth);
    for (int i = 0; i < 6; ++i)
    {
        voiceFaders[static_cast<size_t> (i)].setVisible (isSynth);
        voiceLabels[static_cast<size_t> (i)].setVisible (isSynth);
    }

    detuneSlider.setVisible (isSynth);  detuneLabel.setVisible (isSynth);
    widthSlider.setVisible (isSynth);   widthLabel.setVisible (isSynth);
    resSlider.setVisible (isSynth);     resLabel.setVisible (isSynth);
    driveSlider.setVisible (isSynth);   driveLabel.setVisible (isSynth);
    glideSlider.setVisible (isSynth);   glideLabel.setVisible (isSynth);
    attackSlider.setVisible (isSynth);  attackLabel.setVisible (isSynth);
    releaseSlider.setVisible (isSynth); releaseLabel.setVisible (isSynth);
    synthKeyboard.setVisible (isSynth);

    repaint();
}

void PluginEditor::configureKnob (juce::Slider& slider, juce::Label& label,
                                  const juce::String& text, juce::Colour accent)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 42, 14);
    slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::FontOptions (9.5f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, juce::Colour (0xff94a3b8));
    addAndMakeVisible (label);
}

void PluginEditor::configureVerticalFader (juce::Slider& slider, juce::Label& label,
                                           const juce::String& text, juce::Colour accent)
{
    slider.setSliderStyle (juce::Slider::LinearVertical);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 40, 14);
    slider.setColour (juce::Slider::thumbColourId, accent);
    addChildComponent (slider);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::FontOptions (9.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, juce::Colour (0xff94a3b8));
    addChildComponent (label);
}

void PluginEditor::applyPreset (int idx)
{
    currentPresetIdx = idx;
    auto setP = [&] (const juce::String& id, float val) {
        if (auto* p = audioProcessor.apvts.getParameter (id))
            p->setValueNotifyingHost (val);
    };

    if (idx == 0)
    {
        presetNameBtn.setButtonText ("01 // CATHEDRAL HALO");
        setP ("MORPH_X", 0.35f); setP ("MORPH_Y", 0.45f);
        setP ("SHIMMER_AMT", 0.50f); setP ("AMBIENCE_DECAY", 0.85f); setP ("AMBIENCE_MIX", 0.55f);
        setP ("SYNTH_DETUNE", 0.25f); setP ("SYNTH_RES", 0.20f);
    }
    else if (idx == 1)
    {
        presetNameBtn.setButtonText ("02 // WARM SUB FOUNDATION");
        setP ("MORPH_X", 0.12f); setP ("MORPH_Y", 0.22f);
        setP ("SHIMMER_AMT", 0.20f); setP ("AMBIENCE_DECAY", 0.72f); setP ("AMBIENCE_MIX", 0.40f);
        setP ("SYNTH_DETUNE", 0.15f); setP ("SYNTH_RES", 0.10f);
    }
    else if (idx == 2)
    {
        presetNameBtn.setButtonText ("03 // ANGELIC GLASS CHOIR");
        setP ("MORPH_X", 0.88f); setP ("MORPH_Y", 0.82f);
        setP ("SHIMMER_AMT", 0.78f); setP ("AMBIENCE_DECAY", 0.92f); setP ("AMBIENCE_MIX", 0.68f);
        setP ("SYNTH_DETUNE", 0.45f); setP ("SYNTH_RES", 0.32f);
    }
    else
    {
        presetNameBtn.setButtonText ("04 // INTIMATE ALTAR CALL");
        setP ("MORPH_X", 0.25f); setP ("MORPH_Y", 0.18f);
        setP ("SHIMMER_AMT", 0.35f); setP ("AMBIENCE_DECAY", 0.95f); setP ("AMBIENCE_MIX", 0.62f);
        setP ("SYNTH_DETUNE", 0.30f); setP ("SYNTH_RES", 0.15f);
    }
}

void PluginEditor::timerCallback()
{
    canopyDisplay.advanceAnimation (1.0f / 30.0f);
    xyPad.advanceAnimation (1.0f / 30.0f);

    float outL = audioProcessor.outLevelL.load (std::memory_order_relaxed) * 3.4f;
    float outR = audioProcessor.outLevelR.load (std::memory_order_relaxed) * 3.4f;
    float duck = audioProcessor.duckMeter.load (std::memory_order_relaxed);

    leftDuckMeter.setLevels (duck, duck);
    rightOutMeter.setLevels (outL, outR);

    int activeKey = static_cast<int> (*audioProcessor.apvts.getRawParameterValue ("DRONE_KEY"));
    bool assistOn = *audioProcessor.apvts.getRawParameterValue ("KEY_DETECT_ON") > 0.5f;
    int suggested = audioProcessor.suggestedKeyIndex.load (std::memory_order_relaxed);
    const juce::StringArray keyNames { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    if (! assistOn)
    {
        currentSuggestedKey = -1;
        suggestKeyBtn.setButtonText ("ASSIST OFF");
        suggestKeyBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff12151d));
        suggestKeyBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff475569));
    }
    else if (suggested >= 0 && suggested < 12 && suggested != activeKey)
    {
        currentSuggestedKey = suggested;
        suggestKeyBtn.setButtonText ("USE " + keyNames[suggested] + "?");
        suggestKeyBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xffd97706));
        suggestKeyBtn.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    }
    else
    {
        currentSuggestedKey = -1;
        suggestKeyBtn.setButtonText ("KEY: " + keyNames[activeKey]);
        suggestKeyBtn.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff151821));
        suggestKeyBtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff64748b));
    }

    repaint();
}

void PluginEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient bgGrad (juce::Colour (0xff1c2029), 0.0f, 0.0f,
                                 juce::Colour (0xff151820), 0.0f, bounds.getBottom(), false);
    g.setGradientFill (bgGrad);
    g.fillAll();

    g.setColour (juce::Colours::white.withAlpha (0.01f));
    for (int y = 0; y < getHeight(); y += 2)
        g.drawHorizontalLine (y, 0.0f, bounds.getRight());

    auto header = bounds.removeFromTop (44.0f).reduced (20.0f, 8.0f);
    g.setColour (juce::Colour (0xffe2e8f0));
    g.setFont (juce::FontOptions (14.5f, juce::Font::bold));
    g.drawText ("HALO ENGINE", header.withWidth (130.0f), juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff0b0d12));
    g.drawHorizontalLine (44, 0.0f, bounds.getRight());
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.drawHorizontalLine (45, 0.0f, bounds.getRight());

    auto drawEngravedDivider = [&] (int x, int topY, int bottomY) {
        g.setColour (juce::Colour (0xff0a0c10));
        g.drawVerticalLine (x, static_cast<float> (topY), static_cast<float> (bottomY));
        g.setColour (juce::Colours::white.withAlpha (0.055f));
        g.drawVerticalLine (x + 1, static_cast<float> (topY), static_cast<float> (bottomY));
    };

    auto body = getLocalBounds().withTrimmedTop (45).reduced (16, 12);
    body.removeFromLeft (32 + 12);
    body.removeFromRight (36 + 12);

    if (activeTab == 0)
    {
        int deckTopY = body.getY() + 210 + 14;
        int deckBotY = body.getBottom();

        int totalW = body.getWidth();
        int pod1W  = static_cast<int> (totalW * 0.44f);
        int pod2W  = static_cast<int> (totalW * 0.24f);

        int seam1X = body.getX() + pod1W + 6;
        int seam2X = seam1X + pod2W + 12;

        drawEngravedDivider (seam1X, deckTopY, deckBotY);
        drawEngravedDivider (seam2X, deckTopY, deckBotY);

        g.setColour (juce::Colour (0xff64748b));
        g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
        g.drawText ("DRONE SYNTHESIS", body.getX() + 4, deckTopY, pod1W - 8, 16, juce::Justification::centredLeft);
        g.drawText ("MORPH FIELD",     seam1X + 10,     deckTopY, pod2W - 8, 16, juce::Justification::centredLeft);
        g.drawText ("SHIMMER & SPACE", seam2X + 10,     deckTopY, 180,       16, juce::Justification::centredLeft);
    }
    else
    {
        int topY = body.getY();
        int midY = topY + 235;
        int seamX = body.getX() + static_cast<int> (body.getWidth() * 0.46f);

        drawEngravedDivider (seamX, topY, midY - 8);

        g.setColour (juce::Colour (0xff64748b));
        g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
        g.drawText ("6-VOICE HARMONIC ENGINE MIXER", body.getX() + 4, topY, 260, 16, juce::Justification::centredLeft);
        g.drawText ("OSCILLATOR, FILTER & ENVELOPE", seamX + 14,      topY, 260, 16, juce::Justification::centredLeft);
        g.drawText ("POLYPHONIC SYNTH KEYBOARD (MIDI / CLICK TO PLAY)",
                    body.getX() + 4, body.getBottom() - 122, 380, 16, juce::Justification::centredLeft);
    }
}

void PluginEditor::resized()
{
    auto header = getLocalBounds().removeFromTop (44).reduced (20, 8);

    auto presetArea = header.withTrimmedLeft (135).withWidth (245);
    prevPresetBtn.setBounds (presetArea.removeFromLeft (26));
    presetArea.removeFromLeft (3);
    nextPresetBtn.setBounds (presetArea.removeFromRight (26));
    presetArea.removeFromRight (3);
    presetNameBtn.setBounds (presetArea);

    auto tabArea = header.removeFromRight (216);
    tabMainBtn.setBounds (tabArea.removeFromLeft (104));
    tabArea.removeFromLeft (8);
    tabSynthBtn.setBounds (tabArea);

    auto body = getLocalBounds().withTrimmedTop (45).reduced (16, 12);

    leftDuckMeter.setBounds (body.removeFromLeft (32));
    body.removeFromLeft (12);
    rightOutMeter.setBounds (body.removeFromRight (36));
    body.removeFromRight (12);

    // TAB 0: STAGE VIEW
    auto stageBody = body;
    auto topCanopy = stageBody.removeFromTop (210);
    canopyDisplay.setBounds (topCanopy);

    stageBody.removeFromTop (14);

    int totalW = stageBody.getWidth();
    int pod1W  = static_cast<int> (totalW * 0.44f);
    int pod2W  = static_cast<int> (totalW * 0.24f);

    auto pod1 = stageBody.removeFromLeft (pod1W).reduced (4, 0);
    stageBody.removeFromLeft (12);
    auto pod2 = stageBody.removeFromLeft (pod2W).reduced (4, 0);
    stageBody.removeFromLeft (12);
    auto pod3 = stageBody.reduced (4, 0);

    pod1.removeFromTop (20);
    pod2.removeFromTop (20);
    pod3.removeFromTop (20);

    auto pod1Right = pod1.removeFromRight (84);
    droneLevelLabel.setBounds (pod1Right.removeFromTop (13));
    droneLevelSlider.setBounds (pod1Right.removeFromTop (82));
    pod1Right.removeFromTop (8);
    droneSwitch.setBounds (pod1Right);

    pod1.removeFromRight (10);

    auto assistRow = pod1.removeFromBottom (28);
    keyAssistToggle.setBounds (assistRow.removeFromRight (50));
    assistRow.removeFromRight (6);
    suggestKeyBtn.setBounds (assistRow);

    pod1.removeFromBottom (8);
    keybed.setBounds (pod1);

    xyPad.setBounds (pod2);

    prayerMacroBtn.setBounds (pod3.removeFromBottom (32));
    pod3.removeFromBottom (8);

    int kColW = pod3.getWidth() / 2;
    int kRowH = pod3.getHeight() / 2;

    auto placePodKnob = [&] (juce::Slider& s, juce::Label& l, int col, int row) {
        auto cell = juce::Rectangle<int> (pod3.getX() + col * kColW,
                                          pod3.getY() + row * kRowH,
                                          kColW, kRowH).reduced (2);
        l.setBounds (cell.removeFromTop (12));
        s.setBounds (cell);
    };

    placePodKnob (shimmerSlider, shimmerLabel, 0, 0);
    placePodKnob (decaySlider,   decayLabel,   1, 0);
    placePodKnob (duckSlider,    duckLabel,    0, 1);
    placePodKnob (mixSlider,     mixLabel,     1, 1);

    // TAB 1: SYNTH & VOICING ENGINE LAB
    auto synthBody = body;
    auto synthBottomKeyboard = synthBody.removeFromBottom (104);
    synthBody.removeFromBottom (22);

    auto synthBottomOsc = synthBody.removeFromBottom (115);
    synthBody.removeFromBottom (14);

    oscilloscope.setBounds (synthBottomOsc);

    int leftSplitW = static_cast<int> (synthBody.getWidth() * 0.46f);
    auto faderBank = synthBody.removeFromLeft (leftSplitW).reduced (4, 0);
    synthBody.removeFromLeft (18);
    auto synthKnobsArea = synthBody.reduced (4, 0);

    faderBank.removeFromTop (20);
    synthKnobsArea.removeFromTop (20);

    int faderW = faderBank.getWidth() / 6;
    for (int i = 0; i < 6; ++i)
    {
        auto col = juce::Rectangle<int> (faderBank.getX() + i * faderW, faderBank.getY(), faderW, faderBank.getHeight()).reduced (2);
        voiceLabels[static_cast<size_t> (i)].setBounds (col.removeFromTop (14));
        voiceFaders[static_cast<size_t> (i)].setBounds (col);
    }

    int skColW = synthKnobsArea.getWidth() / 4;
    int skRowH = synthKnobsArea.getHeight() / 2;

    auto placeSynthKnob = [&] (juce::Slider& s, juce::Label& l, int c, int r) {
        auto cell = juce::Rectangle<int> (synthKnobsArea.getX() + c * skColW,
                                          synthKnobsArea.getY() + r * skRowH,
                                          skColW, skRowH).reduced (2);
        l.setBounds (cell.removeFromTop (12));
        s.setBounds (cell);
    };

    placeSynthKnob (detuneSlider,  detuneLabel,  0, 0);
    placeSynthKnob (widthSlider,   widthLabel,   1, 0);
    placeSynthKnob (resSlider,     resLabel,     2, 0);
    placeSynthKnob (driveSlider,   driveLabel,   3, 0);

    placeSynthKnob (glideSlider,   glideLabel,   0, 1);
    placeSynthKnob (attackSlider,  attackLabel,  1, 1);
    placeSynthKnob (releaseSlider, releaseLabel, 2, 1);

    synthKeyboard.setBounds (synthBottomKeyboard);
}