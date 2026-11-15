#include "PluginEditor.h"

PluginEditor::PluginEditor (HaloEngineAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      morphPad (p.apvts),
      oscAPreview (p.wavetableBank, juce::Colour (0xff4FD1C5)),
      oscBPreview (p.wavetableBank, juce::Colour (0xffA78BFA)),
      midiKeyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&customLookAndFeel);
    setSize (920, 580);

    // ==========================================================================
    // HEADER NAVIGATION & PRESET BROWSER
    // ==========================================================================
    for (auto* btn : { &stageTabButton, &synthTabButton })
    {
        btn->setClickingTogglesState (false);
        addAndMakeVisible (btn);
    }
    stageTabButton.onClick = [this] { setActiveTab (0); };
    synthTabButton.onClick = [this] { setActiveTab (1); };

    addAndMakeVisible (prevPresetButton);
    addAndMakeVisible (nextPresetButton);
    addAndMakeVisible (presetSelector);

    const auto& presets = HaloPresetBank::getPresets();
    for (int i = 0; i < static_cast<int> (presets.size()); ++i)
        presetSelector.addItem (presets[static_cast<size_t> (i)].name, i + 1);
    presetSelector.setSelectedId (1, juce::dontSendNotification);
    presetSelector.onChange = [this]
    {
        int sel = presetSelector.getSelectedId() - 1;
        if (sel >= 0)
            loadPresetIndex (sel);
    };

    prevPresetButton.onClick = [this] { loadPresetIndex (currentPresetIdx - 1); };
    nextPresetButton.onClick = [this] { loadPresetIndex (currentPresetIdx + 1); };

    // ==========================================================================
    // GLOBAL ROOT KEY + OCTAVE BAR (ALWAYS ON SCREEN)
    // ==========================================================================
    const std::array<juce::String, 12> noteNames { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    for (int i = 0; i < 12; ++i)
    {
        auto& btn = globalKeyButtons[static_cast<size_t> (i)];
        btn.setButtonText (noteNames[static_cast<size_t> (i)]);
        btn.onClick = [this, i]
        {
            if (auto* param = audioProcessor.apvts.getParameter ("DRONE_KEY"))
                param->setValueNotifyingHost (param->convertTo0to1 (static_cast<float> (i)));
        };
        addAndMakeVisible (btn);
    }

    const std::array<juce::String, 3> octLabels { "MID", "HIGH", "SHIMMER+" };
    for (int o = 0; o < 3; ++o)
    {
        auto& obtn = octaveButtons[static_cast<size_t> (o)];
        obtn.setButtonText (octLabels[static_cast<size_t> (o)]);
        obtn.onClick = [this, o]
        {
            if (auto* param = audioProcessor.apvts.getParameter ("DRONE_OCTAVE"))
                param->setValueNotifyingHost (param->convertTo0to1 (static_cast<float> (o)));
        };
        addAndMakeVisible (obtn);
    }

    dronePowerButton.setClickingTogglesState (true);
    addAndMakeVisible (dronePowerButton);
    dronePowerAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.apvts, "DRONE_ON", dronePowerButton);

    prayerMacroButton.setClickingTogglesState (true);
    prayerMacroButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffF59E0B));
    addAndMakeVisible (prayerMacroButton);
    prayerMacroAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.apvts, "PRAYER_MACRO", prayerMacroButton);

    auto configureKnob = [this] (juce::Slider& slider, juce::Label& label, const juce::String& text, juce::Colour accent)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 46, 13);
        slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
        slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffCBD5E1));
        slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (slider);

        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::FontOptions (9.5f, juce::Font::bold));
        label.setColour (juce::Label::textColourId, juce::Colour (0xff94A3B8));
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
    };

    // ==========================================================================
    // TAB 0: STAGE VIEW CONTROLS
    // ==========================================================================
    addAndMakeVisible (morphPad);

    configureKnob (droneLevelSlider, droneLevelLabel, "DRONE LVL",   juce::Colour (0xff4FD1C5));
    configureKnob (shimmerSlider,    shimmerLabel,    "SHIMMER OCT", juce::Colour (0xffA78BFA));
    configureKnob (decaySlider,      decayLabel,      "REVERB TIME", juce::Colour (0xff38BDF8));
    configureKnob (mixSlider,        mixLabel,        "REVERB MIX",  juce::Colour (0xff4FD1C5));

    configureKnob (spaceWarpSlider,  spaceWarpLabel,  "CLOUD SIZE",  juce::Colour (0xffA78BFA));
    configureKnob (spaceModSlider,   spaceModLabel,   "CHORUS MOD",  juce::Colour (0xff38BDF8));
    configureKnob (spaceDampSlider,  spaceDampLabel,  "AIR DAMP",    juce::Colour (0xff94A3B8));

    droneLevelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "DRONE_LEVEL",    droneLevelSlider);
    shimmerAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SHIMMER_AMT",    shimmerSlider);
    decayAtt      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "AMBIENCE_DECAY", decaySlider);
    mixAtt        = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "AMBIENCE_MIX",   mixSlider);

    spaceWarpAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SPACE_WARP",     spaceWarpSlider);
    spaceModAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SPACE_MOD",      spaceModSlider);
    spaceDampAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SPACE_DAMP",     spaceDampSlider);

    // ==========================================================================
    // TAB 1: SYNTH ENGINE CONTROLS
    // ==========================================================================
    addAndMakeVisible (oscAPreview);
    addAndMakeVisible (oscBPreview);

    juce::StringArray wtNames   { "01 // ANALOG SAW-TRI", "02 // VOCAL CHOIR", "03 // GLASS BELL", "04 // PWM ENSEMBLE", "05 // BOWED STRINGS" };
    juce::StringArray warpModes { "WARP: OFF", "PHASE BEND", "HARD SYNC", "FM (B -> A)" };

    for (int i = 0; i < wtNames.size(); ++i)
    {
        oscATableBox.addItem (wtNames[i], i + 1);
        oscBTableBox.addItem (wtNames[i], i + 1);
    }
    for (int i = 0; i < warpModes.size(); ++i)
        oscAWarpModeBox.addItem (warpModes[i], i + 1);

    addAndMakeVisible (oscATableBox);
    addAndMakeVisible (oscAWarpModeBox);
    addAndMakeVisible (oscBTableBox);

    oscATableAtt    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "OSC_A_TABLE",     oscATableBox);
    oscAWarpModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "OSC_A_WARP_MODE", oscAWarpModeBox);
    oscBTableAtt    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "OSC_B_TABLE",     oscBTableBox);

    configureKnob (oscAPosSlider,   oscAPosLabel,   "WT POS",    juce::Colour (0xff4FD1C5));
    configureKnob (oscAWarpSlider,  oscAWarpLabel,  "WARP AMT",  juce::Colour (0xff4FD1C5));
    configureKnob (oscALevelSlider, oscALevelLabel, "OSC A LVL", juce::Colour (0xff4FD1C5));

    configureKnob (oscBPosSlider,   oscBPosLabel,   "WT POS",    juce::Colour (0xffA78BFA));
    configureKnob (oscBOctSlider,   oscBOctLabel,   "OCTAVE",    juce::Colour (0xffA78BFA));
    configureKnob (oscBLevelSlider, oscBLevelLabel, "OSC B LVL", juce::Colour (0xffA78BFA));
    configureKnob (airNoiseSlider,  airNoiseLabel,  "AIR BREATH",juce::Colour (0xff38BDF8));

    oscAPosAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "OSC_A_POS",   oscAPosSlider);
    oscAWarpAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "OSC_A_WARP",  oscAWarpSlider);
    oscALevelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "OSC_A_LEVEL", oscALevelSlider);

    oscBPosAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "OSC_B_POS",   oscBPosSlider);
    oscBOctAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "OSC_B_OCT",   oscBOctSlider);
    oscBLevelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "OSC_B_LEVEL", oscBLevelSlider);
    airNoiseAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "AIR_NOISE",   airNoiseSlider);

    juce::StringArray fModes  { "LP 24dB LADDER", "LP 12dB OB-6", "BP FORMANT", "NOTCH PHASER" };
    juce::StringArray lShapes { "SINE", "TRIANGLE", "WANDER" };

    for (int i = 0; i < fModes.size(); ++i)
        filterModeBox.addItem (fModes[i], i + 1);
    for (int i = 0; i < lShapes.size(); ++i)
    {
        lfo1ShapeBox.addItem (lShapes[i], i + 1);
        lfo2ShapeBox.addItem (lShapes[i], i + 1);
    }

    addAndMakeVisible (filterModeBox);
    addAndMakeVisible (lfo1ShapeBox);
    addAndMakeVisible (lfo2ShapeBox);

    filterModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "FILTER_MODE", filterModeBox);
    lfo1ShapeAtt  = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "LFO1_SHAPE",  lfo1ShapeBox);
    lfo2ShapeAtt  = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "LFO2_SHAPE",  lfo2ShapeBox);

    configureKnob (filterResSlider, filterResLabel, "RESONANCE",  juce::Colour (0xff38BDF8));
    configureKnob (filterEnvSlider, filterEnvLabel, "ENV2 -> CUT",juce::Colour (0xff38BDF8));
    configureKnob (driveSlider,     driveLabel,     "SATURATION", juce::Colour (0xffF59E0B));

    configureKnob (lfo1RateSlider,  lfo1RateLabel,  "LFO1 RATE",  juce::Colour (0xff4FD1C5));
    configureKnob (lfo1WtSlider,    lfo1WtLabel,    "LFO1 -> WT", juce::Colour (0xff4FD1C5));
    configureKnob (lfo2RateSlider,  lfo2RateLabel,  "LFO2 RATE",  juce::Colour (0xffA78BFA));
    configureKnob (lfo2CutSlider,   lfo2CutLabel,   "LFO2 -> CUT",juce::Colour (0xffA78BFA));

    filterResAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SYNTH_RES",      filterResSlider);
    filterEnvAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "FILTER_ENV_AMT", filterEnvSlider);
    driveAtt     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SYNTH_DRIVE",    driveSlider);

    lfo1RateAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "LFO1_RATE",      lfo1RateSlider);
    lfo1WtAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "LFO1_WT_AMT",    lfo1WtSlider);
    lfo2RateAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "LFO2_RATE",      lfo2RateSlider);
    lfo2CutAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "LFO2_CUT_AMT",   lfo2CutSlider);

    const std::array<juce::String, 6> voiceParamIDs { "VOICE_SUB", "VOICE_ROOT", "VOICE_FIFTH", "VOICE_OCT", "VOICE_NINTH", "VOICE_HIGH" };
    const std::array<juce::String, 6> voiceNames    { "LOW", "ROOT", "5TH", "OCT", "9TH", "HI 5TH" };
    const std::array<juce::Colour, 6> voiceColors   {
        juce::Colour (0xff38BDF8), juce::Colour (0xff4FD1C5), juce::Colour (0xff4FD1C5),
        juce::Colour (0xffA78BFA), juce::Colour (0xffA78BFA), juce::Colour (0xffF59E0B)
    };

    for (size_t i = 0; i < 6; ++i)
    {
        auto& fader = voiceFaders[i];
        fader.setSliderStyle (juce::Slider::LinearVertical);
        fader.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        fader.setColour (juce::Slider::thumbColourId, voiceColors[i]);
        addAndMakeVisible (fader);

        auto& lbl = voiceLabels[i];
        lbl.setText (voiceNames[i], juce::dontSendNotification);
        lbl.setFont (juce::FontOptions (9.0f, juce::Font::bold));
        lbl.setColour (juce::Label::textColourId, juce::Colour (0xff94A3B8));
        lbl.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (lbl);

        voiceAtts[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            audioProcessor.apvts, voiceParamIDs[i], fader);
    }

    configureKnob (detuneSlider,  detuneLabel,  "UNISON",  juce::Colour (0xffA78BFA));
    configureKnob (widthSlider,   widthLabel,   "WIDTH",   juce::Colour (0xff38BDF8));
    configureKnob (glideSlider,   glideLabel,   "GLIDE",   juce::Colour (0xff4FD1C5));
    configureKnob (attackSlider,  attackLabel,  "ATTACK",  juce::Colour (0xff4FD1C5));
    configureKnob (releaseSlider, releaseLabel, "RELEASE", juce::Colour (0xff4FD1C5));

    detuneAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SYNTH_DETUNE",  detuneSlider);
    widthAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SYNTH_WIDTH",   widthSlider);
    glideAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SYNTH_GLIDE",   glideSlider);
    attackAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SYNTH_ATTACK",  attackSlider);
    releaseAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SYNTH_RELEASE", releaseSlider);

    midiKeyboard.setKeyWidth (21.0f);
    midiKeyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xffE2E8F0));
    midiKeyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff121824));
    midiKeyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour (0xff4FD1C5));
    addAndMakeVisible (midiKeyboard);

    loadPresetIndex (0);
    setActiveTab (0);
    startTimerHz (30);
}

PluginEditor::~PluginEditor()
{
    setLookAndFeel (nullptr);
}

void PluginEditor::loadPresetIndex (int newIndex)
{
    const auto& presets = HaloPresetBank::getPresets();
    int count = static_cast<int> (presets.size());
    if (count == 0) return;

    currentPresetIdx = ((newIndex % count) + count) % count;
    presetSelector.setSelectedId (currentPresetIdx + 1, juce::dontSendNotification);
    HaloPresetBank::applyPreset (audioProcessor.apvts, currentPresetIdx);
    morphPad.repaint();
}

void PluginEditor::setActiveTab (int tabIndex)
{
    currentTab = tabIndex;

    stageTabButton.setColour (juce::TextButton::buttonColourId,
                              currentTab == 0 ? juce::Colour (0xff4FD1C5) : juce::Colour (0xff161C27));
    stageTabButton.setColour (juce::TextButton::textColourOffId,
                              currentTab == 0 ? juce::Colour (0xff0B0F17) : juce::Colour (0xff94A3B8));

    synthTabButton.setColour (juce::TextButton::buttonColourId,
                              currentTab == 1 ? juce::Colour (0xffA78BFA) : juce::Colour (0xff161C27));
    synthTabButton.setColour (juce::TextButton::textColourOffId,
                              currentTab == 1 ? juce::Colour (0xff0B0F17) : juce::Colour (0xff94A3B8));

    bool isStage = (currentTab == 0);
    morphPad.setVisible (isStage);

    for (auto* c : std::initializer_list<juce::Component*> {
        &droneLevelSlider, &shimmerSlider, &decaySlider, &mixSlider,
        &spaceWarpSlider, &spaceModSlider, &spaceDampSlider,
        &droneLevelLabel, &shimmerLabel, &decayLabel, &mixLabel,
        &spaceWarpLabel, &spaceModLabel, &spaceDampLabel })
    {
        c->setVisible (isStage);
    }

    bool isSynth = (currentTab == 1);

    for (auto* c : std::initializer_list<juce::Component*> {
        &oscAPreview, &oscBPreview,
        &oscATableBox, &oscAWarpModeBox, &oscAPosSlider, &oscAWarpSlider, &oscALevelSlider,
        &oscAPosLabel, &oscAWarpLabel, &oscALevelLabel,
        &oscBTableBox, &oscBPosSlider, &oscBOctSlider, &oscBLevelSlider, &airNoiseSlider,
        &oscBPosLabel, &oscBOctLabel, &oscBLevelLabel, &airNoiseLabel,
        &filterModeBox, &lfo1ShapeBox, &lfo2ShapeBox,
        &filterResSlider, &filterEnvSlider, &driveSlider,
        &lfo1RateSlider, &lfo1WtSlider, &lfo2RateSlider, &lfo2CutSlider,
        &filterResLabel, &filterEnvLabel, &driveLabel,
        &lfo1RateLabel, &lfo1WtLabel, &lfo2RateLabel, &lfo2CutLabel,
        &detuneSlider, &widthSlider, &glideSlider, &attackSlider, &releaseSlider,
        &detuneLabel, &widthLabel, &glideLabel, &attackLabel, &releaseLabel,
        &midiKeyboard })
    {
        c->setVisible (isSynth);
    }

    for (size_t i = 0; i < 6; ++i)
    {
        voiceFaders[i].setVisible (isSynth);
        voiceLabels[i].setVisible (isSynth);
    }

    repaint();
}

void PluginEditor::timerCallback()
{
    int activeKey = static_cast<int> (*audioProcessor.apvts.getRawParameterValue ("DRONE_KEY"));
    int activeOct = static_cast<int> (*audioProcessor.apvts.getRawParameterValue ("DRONE_OCTAVE"));
    bool droneOn  = *audioProcessor.apvts.getRawParameterValue ("DRONE_ON") > 0.5f;
    bool prayerOn = *audioProcessor.apvts.getRawParameterValue ("PRAYER_MACRO") > 0.5f;

    for (int i = 0; i < 12; ++i)
    {
        auto& btn = globalKeyButtons[static_cast<size_t> (i)];
        if (i == activeKey)
        {
            btn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff4FD1C5));
            btn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff0B0F17));
        }
        else
        {
            btn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff151B26));
            btn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff94A3B8));
        }
    }

    for (int o = 0; o < 3; ++o)
    {
        auto& obtn = octaveButtons[static_cast<size_t> (o)];
        if (o == activeOct)
        {
            obtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffA78BFA));
            obtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff0B0F17));
        }
        else
        {
            obtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff151B26));
            obtn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff94A3B8));
        }
    }

    dronePowerButton.setButtonText (droneOn ? "DRONE: ON" : "DRONE: OFF");
    dronePowerButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff4FD1C5));
    prayerMacroButton.setButtonText (prayerOn ? "SWELL: ON" : "SWELL");

    if (currentTab == 0)
    {
        morphPad.repaint();
    }
    else
    {
        int oscATab = static_cast<int> (*audioProcessor.apvts.getRawParameterValue ("OSC_A_TABLE"));
        int oscAWm  = static_cast<int> (*audioProcessor.apvts.getRawParameterValue ("OSC_A_WARP_MODE"));
        float oscAW = *audioProcessor.apvts.getRawParameterValue ("OSC_A_WARP");
        float livePosA = audioProcessor.liveOscAPos.load (std::memory_order_relaxed);
        oscAPreview.updateState (oscATab, livePosA, oscAW, oscAWm);

        int oscBTab = static_cast<int> (*audioProcessor.apvts.getRawParameterValue ("OSC_B_TABLE"));
        float livePosB = audioProcessor.liveOscBPos.load (std::memory_order_relaxed);
        oscBPreview.updateState (oscBTab, livePosB, 0.0f, 0);
    }

    repaint();
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0B0F17));

    // Top Header Bar
    g.setColour (juce::Colour (0xff101622));
    g.fillRect (0, 0, getWidth(), 50);
    g.setColour (juce::Colour (0xff1E2636));
    g.drawHorizontalLine (50, 0.0f, static_cast<float> (getWidth()));

    g.setFont (juce::FontOptions (17.0f, juce::Font::bold));
    g.setColour (juce::Colour (0xffF8FAFC));
    g.drawText ("HALO ENGINE", 16, 7, 180, 20, juce::Justification::left);

    g.setFont (juce::FontOptions (9.0f, juce::Font::bold));
    g.setColour (juce::Colour (0xff4FD1C5));
    g.drawText ("AMBIENT WAVETABLE & SHIMMER SYNTH", 17, 28, 250, 14, juce::Justification::left);

    // Output Stereo Meters
    float lvlL = juce::jlimit (0.0f, 1.0f, audioProcessor.outLevelL.load (std::memory_order_relaxed) * 2.4f);
    float lvlR = juce::jlimit (0.0f, 1.0f, audioProcessor.outLevelR.load (std::memory_order_relaxed) * 2.4f);

    int meterX = getWidth() - 94;
    g.setColour (juce::Colour (0xff18202F));
    g.fillRoundedRectangle (static_cast<float> (meterX), 15.0f, 78.0f, 6.0f, 3.0f);
    g.fillRoundedRectangle (static_cast<float> (meterX), 25.0f, 78.0f, 6.0f, 3.0f);

    g.setColour (juce::Colour (0xff4FD1C5));
    g.fillRoundedRectangle (static_cast<float> (meterX), 15.0f, 78.0f * lvlL, 6.0f, 3.0f);
    g.fillRoundedRectangle (static_cast<float> (meterX), 25.0f, 78.0f * lvlR, 6.0f, 3.0f);

    // Global Root Key + Octave Bar (y = 56..92)
    g.setColour (juce::Colour (0xff101622));
    g.fillRoundedRectangle (16.0f, 56.0f, static_cast<float> (getWidth() - 32), 36.0f, 8.0f);
    g.setColour (juce::Colour (0xff1E2636));
    g.drawRoundedRectangle (16.0f, 56.0f, static_cast<float> (getWidth() - 32), 36.0f, 8.0f, 1.0f);

    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    g.setColour (juce::Colour (0xff94A3B8));
    g.drawText ("ROOT KEY", 26, 56, 68, 36, juce::Justification::left);

    int topOffset = 100;

    if (currentTab == 0)
    {
        // ======================================================================
        // TAB 0: STAGE VIEW (MORPH PAD + REVERB & SHIMMER)
        // ======================================================================
        g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
        g.setColour (juce::Colour (0xff94A3B8));
        g.drawText ("2D TIMBRE & SHIMMER MORPH PAD", 20, topOffset, 260, 16, juce::Justification::left);

        int rightX = getWidth() - 356;
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (static_cast<float> (rightX), static_cast<float> (topOffset), 340.0f, static_cast<float> (getHeight() - topOffset - 14), 8.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (static_cast<float> (rightX), static_cast<float> (topOffset), 340.0f, static_cast<float> (getHeight() - topOffset - 14), 8.0f, 1.0f);

        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xffA78BFA));
        g.drawText ("REVERB & SHIMMER", rightX + 16, topOffset + 12, 240, 16, juce::Justification::left);
    }
    else
    {
        // ======================================================================
        // TAB 1: COMPACT SYNTH ENGINE VIEW
        // ======================================================================
        int cardTop = topOffset;
        int row1H = 156;
        int row2Top = cardTop + row1H + 8;
        int row2H = 118;
        int row3Top = row2Top + row2H + 8;
        int row3H = 106;

        // OSC A Card
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (16.0f, static_cast<float> (cardTop), 440.0f, static_cast<float> (row1H), 8.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (16.0f, static_cast<float> (cardTop), 440.0f, static_cast<float> (row1H), 8.0f, 1.0f);
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xff4FD1C5));
        g.drawText ("OSC A // WAVETABLE & WARP", 28, cardTop + 6, 240, 14, juce::Justification::left);

        // OSC B + Air Breath Card
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (464.0f, static_cast<float> (cardTop), 440.0f, static_cast<float> (row1H), 8.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (464.0f, static_cast<float> (cardTop), 440.0f, static_cast<float> (row1H), 8.0f, 1.0f);
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xffA78BFA));
        g.drawText ("OSC B // SHIMMER LAYER & AIR", 476, cardTop + 6, 250, 14, juce::Justification::left);

        // Row 2 Left: Filter + Saturation
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (16.0f, static_cast<float> (row2Top), 440.0f, static_cast<float> (row2H), 8.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (16.0f, static_cast<float> (row2Top), 440.0f, static_cast<float> (row2H), 8.0f, 1.0f);
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xff38BDF8));
        g.drawText ("ZDF ANALOG FILTER & SATURATION", 28, row2Top + 6, 260, 14, juce::Justification::left);

        // Row 2 Right: LFO 1 & LFO 2
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (464.0f, static_cast<float> (row2Top), 440.0f, static_cast<float> (row2H), 8.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (464.0f, static_cast<float> (row2Top), 440.0f, static_cast<float> (row2H), 8.0f, 1.0f);
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xffF59E0B));
        g.drawText ("MODULATION // LFO 1 (WT) & LFO 2 (CUTOFF)", 476, row2Top + 6, 300, 14, juce::Justification::left);

        // Row 3 Left: 6-Voice Harmonic Mixer
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (16.0f, static_cast<float> (row3Top), 286.0f, static_cast<float> (row3H), 8.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (16.0f, static_cast<float> (row3Top), 286.0f, static_cast<float> (row3H), 8.0f, 1.0f);
        g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
        g.setColour (juce::Colour (0xff4FD1C5));
        g.drawText ("HARMONIC VOICING MIXER", 26, row3Top + 5, 220, 13, juce::Justification::left);

        // Row 3 Middle: Unison, Glide & Envelope
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (310.0f, static_cast<float> (row3Top), 368.0f, static_cast<float> (row3H), 8.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (310.0f, static_cast<float> (row3Top), 368.0f, static_cast<float> (row3H), 8.0f, 1.0f);
        g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
        g.setColour (juce::Colour (0xffA78BFA));
        g.drawText ("UNISON, GLIDE & ENVELOPE", 320, row3Top + 5, 240, 13, juce::Justification::left);

        // Row 3 Right: Master Oscilloscope
        juce::Rectangle<float> scopeRect (686.0f, static_cast<float> (row3Top), 218.0f, static_cast<float> (row3H));
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (scopeRect, 8.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (scopeRect, 8.0f, 1.0f);

        g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
        g.setColour (juce::Colour (0xff4FD1C5));
        g.drawText ("OUTPUT SCOPE", 696, row3Top + 5, 140, 13, juce::Justification::left);

        juce::Path scopePath;
        float midY = scopeRect.getCentreY() + 6.0f;
        float ampY = scopeRect.getHeight() * 0.34f;
        float startX = scopeRect.getX() + 10.0f;
        float stepX  = (scopeRect.getWidth() - 20.0f) / static_cast<float> (HaloEngineAudioProcessor::scopeSize - 1);

        for (int i = 0; i < HaloEngineAudioProcessor::scopeSize; ++i)
        {
            float sample = audioProcessor.scopeSamples[static_cast<size_t> (i)].load (std::memory_order_relaxed);
            float px = startX + i * stepX;
            float py = midY - sample * ampY;
            if (i == 0) scopePath.startNewSubPath (px, py);
            else scopePath.lineTo (px, py);
        }

        g.setColour (juce::Colour (0xff4FD1C5).withAlpha (0.28f));
        g.strokePath (scopePath, juce::PathStrokeType (3.5f));
        g.setColour (juce::Colour (0xff4FD1C5));
        g.strokePath (scopePath, juce::PathStrokeType (1.6f));
    }
}

void PluginEditor::resized()
{
    // Top Header Controls
    stageTabButton.setBounds   (296, 11, 86, 28);
    synthTabButton.setBounds   (388, 11, 112, 28);

    prevPresetButton.setBounds (514, 11, 24, 28);
    presetSelector.setBounds   (540, 11, 186, 28);
    nextPresetButton.setBounds (728, 11, 24, 28);

    // Always-Visible Global Root Key + Octave Bar (y = 56..92)
    int kx = 92;
    for (int i = 0; i < 12; ++i)
    {
        globalKeyButtons[static_cast<size_t> (i)].setBounds (kx + i * 36, 61, 33, 26);
    }

    int ox = kx + 12 * 36 + 12;
    octaveButtons[0].setBounds (ox,       61, 44, 26);
    octaveButtons[1].setBounds (ox + 47,  61, 50, 26);
    octaveButtons[2].setBounds (ox + 100, 61, 74, 26);

    dronePowerButton.setBounds  (getWidth() - 178, 61, 82, 26);
    prayerMacroButton.setBounds (getWidth() - 90,  61, 68, 26);

    int topOffset = 100;

    auto placeKnob = [] (juce::Slider& s, juce::Label& l, int x, int y, int w, int h)
    {
        l.setBounds (x, y, w, 13);
        s.setBounds (x, y + 13, w, h);
    };

    // ==========================================================================
    // TAB 0: STAGE VIEW LAYOUT
    // ==========================================================================
    int rightX = getWidth() - 356;
    morphPad.setBounds (16, topOffset + 20, rightX - 28, getHeight() - topOffset - 34);

    int kW = 98;
    int kH = 92;
    int r1Y = topOffset + 46;
    int r2Y = r1Y + kH + 30;
    int r3Y = r2Y + kH + 30;

    placeKnob (droneLevelSlider, droneLevelLabel, rightX + 18,  r1Y, kW, kH);
    placeKnob (shimmerSlider,    shimmerLabel,    rightX + 121, r1Y, kW, kH);
    placeKnob (decaySlider,      decayLabel,      rightX + 224, r1Y, kW, kH);

    placeKnob (spaceWarpSlider,  spaceWarpLabel,  rightX + 18,  r2Y, kW, kH);
    placeKnob (spaceModSlider,   spaceModLabel,   rightX + 121, r2Y, kW, kH);
    placeKnob (spaceDampSlider,  spaceDampLabel,  rightX + 224, r2Y, kW, kH);

    placeKnob (mixSlider,        mixLabel,        rightX + 121, r3Y, kW, kH);

    // ==========================================================================
    // TAB 1: COMPACT SYNTH ENGINE LAYOUT
    // ==========================================================================
    int cardTop = topOffset;
    int row1H = 156;
    int row2Top = cardTop + row1H + 8;
    int row2H = 118;
    int row3Top = row2Top + row2H + 8;
    int row3H = 106;

    // OSC A Card
    oscATableBox.setBounds    (28, cardTop + 24, 186, 22);
    oscAWarpModeBox.setBounds (28, cardTop + 50, 186, 22);
    oscAPreview.setBounds     (28, cardTop + 78, 186, 68);

    placeKnob (oscAPosSlider,   oscAPosLabel,   224, cardTop + 34, 72, 76);
    placeKnob (oscAWarpSlider,  oscAWarpLabel,  300, cardTop + 34, 72, 76);
    placeKnob (oscALevelSlider, oscALevelLabel, 376, cardTop + 34, 72, 76);

    // OSC B Card
    oscBTableBox.setBounds (476, cardTop + 24, 186, 22);
    oscBPreview.setBounds  (476, cardTop + 52, 186, 94);

    placeKnob (oscBPosSlider,   oscBPosLabel,   668, cardTop + 34, 56, 66);
    placeKnob (oscBOctSlider,   oscBOctLabel,   726, cardTop + 34, 56, 66);
    placeKnob (oscBLevelSlider, oscBLevelLabel, 784, cardTop + 34, 56, 66);
    placeKnob (airNoiseSlider,  airNoiseLabel,  842, cardTop + 34, 56, 66);

    // Row 2 Left: Filter + Saturation
    filterModeBox.setBounds (28, row2Top + 26, 154, 24);
    placeKnob (filterResSlider, filterResLabel, 194, row2Top + 22, 82, 72);
    placeKnob (filterEnvSlider, filterEnvLabel, 280, row2Top + 22, 82, 72);
    placeKnob (driveSlider,     driveLabel,     366, row2Top + 22, 82, 72);

    // Row 2 Right: LFO 1 & LFO 2
    lfo1ShapeBox.setBounds (476, row2Top + 26, 102, 24);
    placeKnob (lfo1RateSlider, lfo1RateLabel, 582, row2Top + 22, 52, 66);
    placeKnob (lfo1WtSlider,   lfo1WtLabel,   635, row2Top + 22, 52, 66);

    lfo2ShapeBox.setBounds (694, row2Top + 26, 102, 24);
    placeKnob (lfo2RateSlider, lfo2RateLabel, 800, row2Top + 22, 52, 66);
    placeKnob (lfo2CutSlider,  lfo2CutLabel,  853, row2Top + 22, 52, 66);

    // Row 3 Left: 6-Voice Harmonic Mixer Faders
    for (size_t i = 0; i < 6; ++i)
    {
        int fx = 24 + static_cast<int> (i) * 45;
        voiceLabels[i].setBounds (fx, row3Top + 20, 40, 12);
        voiceFaders[i].setBounds (fx + 6, row3Top + 33, 28, row3H - 38);
    }

    // Row 3 Middle: Unison, Glide & Envelope
    int gStartX = 316;
    int gW = 70;
    placeKnob (detuneSlider,  detuneLabel,  gStartX + gW * 0, row3Top + 22, gW, 64);
    placeKnob (widthSlider,   widthLabel,   gStartX + gW * 1, row3Top + 22, gW, 64);
    placeKnob (glideSlider,   glideLabel,   gStartX + gW * 2, row3Top + 22, gW, 64);
    placeKnob (attackSlider,  attackLabel,  gStartX + gW * 3, row3Top + 22, gW, 64);
    placeKnob (releaseSlider, releaseLabel, gStartX + gW * 4, row3Top + 22, gW, 64);

    // Compact Keyboard along bottom
    int kbY = row3Top + row3H + 8;
    midiKeyboard.setBounds (16, kbY, getWidth() - 32, getHeight() - kbY - 8);
}
