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
    setSize (1040, 680);

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

    keyAssistToggle.setClickingTogglesState (true);
    addAndMakeVisible (keyAssistToggle);
    keyAssistAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.apvts, "KEY_DETECT_ON", keyAssistToggle);

    applyKeyButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffF59E0B));
    applyKeyButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff0B0F17));
    applyKeyButton.onClick = [this]
    {
        if (detectedKeyCache >= 0 && detectedKeyCache < 12)
        {
            if (auto* param = audioProcessor.apvts.getParameter ("DRONE_KEY"))
                param->setValueNotifyingHost (param->convertTo0to1 (static_cast<float> (detectedKeyCache)));
        }
    };
    addChildComponent (applyKeyButton);

    auto configureKnob = [this] (juce::Slider& slider, juce::Label& label, const juce::String& text, juce::Colour accent)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 52, 14);
        slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
        slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffCBD5E1));
        slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (slider);

        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::Font (10.0f, juce::Font::bold));
        label.setColour (juce::Label::textColourId, juce::Colour (0xff94A3B8));
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
    };

    // ==========================================================================
    // TAB 0: STAGE VIEW CONTROLS
    // ==========================================================================
    const std::array<juce::String, 12> noteNames { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    for (int i = 0; i < 12; ++i)
    {
        auto& btn = keyButtons[static_cast<size_t> (i)];
        btn.setButtonText (noteNames[static_cast<size_t> (i)]);
        btn.onClick = [this, i]
        {
            if (auto* param = audioProcessor.apvts.getParameter ("DRONE_KEY"))
                param->setValueNotifyingHost (param->convertTo0to1 (static_cast<float> (i)));
        };
        addAndMakeVisible (btn);
    }

    dronePowerButton.setClickingTogglesState (true);
    addAndMakeVisible (dronePowerButton);
    dronePowerAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.apvts, "DRONE_ON", dronePowerButton);

    prayerMacroButton.setClickingTogglesState (true);
    prayerMacroButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffF59E0B));
    addAndMakeVisible (prayerMacroButton);
    prayerMacroAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.apvts, "PRAYER_MACRO", prayerMacroButton);

    addAndMakeVisible (morphPad);

    configureKnob (droneLevelSlider, droneLevelLabel, "DRONE LVL",   juce::Colour (0xff4FD1C5));
    configureKnob (shimmerSlider,    shimmerLabel,    "SHIMMER OCT", juce::Colour (0xffA78BFA));
    configureKnob (decaySlider,      decayLabel,      "SPACE DECAY", juce::Colour (0xff38BDF8));
    configureKnob (autoDuckSlider,   autoDuckLabel,   "BAND DUCK",   juce::Colour (0xffF59E0B));
    configureKnob (mixSlider,        mixLabel,        "WET MIX",     juce::Colour (0xff4FD1C5));

    configureKnob (spaceWarpSlider,  spaceWarpLabel,  "CLOUD WARP",  juce::Colour (0xffA78BFA));
    configureKnob (spaceModSlider,   spaceModLabel,   "TANK MOD",    juce::Colour (0xff38BDF8));
    configureKnob (spaceDampSlider,  spaceDampLabel,  "AIR DAMP",    juce::Colour (0xff94A3B8));

    droneLevelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "DRONE_LEVEL",    droneLevelSlider);
    shimmerAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SHIMMER_AMT",    shimmerSlider);
    decayAtt      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "AMBIENCE_DECAY", decaySlider);
    autoDuckAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "AUTO_DUCK",      autoDuckSlider);
    mixAtt        = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "AMBIENCE_MIX",   mixSlider);

    spaceWarpAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SPACE_WARP",     spaceWarpSlider);
    spaceModAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SPACE_MOD",      spaceModSlider);
    spaceDampAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SPACE_DAMP",     spaceDampSlider);

    // ==========================================================================
    // TAB 1: SERUM / VITAL / SUPERMASSIVE SYNTH ENGINE CONTROLS
    // ==========================================================================
    addAndMakeVisible (oscAPreview);
    addAndMakeVisible (oscBPreview);

    juce::StringArray wtNames { "01 // ANALOG SAW-TRI", "02 // VOCAL CHOIR", "03 // GLASS BELL", "04 // PWM ENSEMBLE", "05 // BOWED STRINGS" };
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
    configureKnob (airNoiseSlider,  airNoiseLabel,  "FELT AIR",  juce::Colour (0xff38BDF8));

    oscAPosAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "OSC_A_POS",   oscAPosSlider);
    oscAWarpAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "OSC_A_WARP",  oscAWarpSlider);
    oscALevelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "OSC_A_LEVEL", oscALevelSlider);

    oscBPosAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "OSC_B_POS",   oscBPosSlider);
    oscBOctAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "OSC_B_OCT",   oscBOctSlider);
    oscBLevelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "OSC_B_LEVEL", oscBLevelSlider);
    airNoiseAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "AIR_NOISE",   airNoiseSlider);

    // Filter + LFO 1 & LFO 2 Controls
    juce::StringArray fModes { "LP 24dB LADDER", "LP 12dB OB-6", "BP FORMANT", "NOTCH PHASER" };
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
    configureKnob (driveSlider,     driveLabel,     "TUBE DRIVE", juce::Colour (0xffF59E0B));

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

    // 6-Voice Harmonic Drone Mixer Faders
    const std::array<juce::String, 6> voiceParamIDs { "VOICE_SUB", "VOICE_ROOT", "VOICE_FIFTH", "VOICE_OCT", "VOICE_NINTH", "VOICE_HIGH" };
    const std::array<juce::String, 6> voiceNames    { "SUB", "ROOT", "5TH", "OCT", "9TH", "HI 5TH" };
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
        lbl.setFont (juce::Font (9.5f, juce::Font::bold));
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

    midiKeyboard.setKeyWidth (24.0f);
    midiKeyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xffE2E8F0));
    midiKeyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff121824));
    midiKeyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour (0xff4FD1C5));
    addAndMakeVisible (midiKeyboard);

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

    for (auto& b : keyButtons) b.setVisible (isStage);
    dronePowerButton.setVisible (isStage);
    prayerMacroButton.setVisible (isStage);
    morphPad.setVisible (isStage);

    for (auto* c : std::initializer_list<juce::Component*> {
        &droneLevelSlider, &shimmerSlider, &decaySlider, &autoDuckSlider, &mixSlider,
        &spaceWarpSlider, &spaceModSlider, &spaceDampSlider,
        &droneLevelLabel, &shimmerLabel, &decayLabel, &autoDuckLabel, &mixLabel,
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
    bool droneOn  = *audioProcessor.apvts.getRawParameterValue ("DRONE_ON") > 0.5f;
    bool prayerOn = *audioProcessor.apvts.getRawParameterValue ("PRAYER_MACRO") > 0.5f;
    bool assistOn = *audioProcessor.apvts.getRawParameterValue ("KEY_DETECT_ON") > 0.5f;

    keyAssistToggle.setButtonText (assistOn ? "KEY ASSIST: ON" : "KEY ASSIST: OFF");

    detectedKeyCache = audioProcessor.suggestedKeyIndex.load (std::memory_order_relaxed);
    bool showBanner = assistOn && (detectedKeyCache >= 0 && detectedKeyCache < 12 && detectedKeyCache != activeKey);
    applyKeyButton.setVisible (showBanner);

    if (currentTab == 0)
    {
        for (int i = 0; i < 12; ++i)
        {
            auto& btn = keyButtons[static_cast<size_t> (i)];
            if (i == activeKey)
            {
                btn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff4FD1C5));
                btn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff0B0F17));
            }
            else if (showBanner && i == detectedKeyCache)
            {
                btn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffF59E0B).withAlpha (0.28f));
                btn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffFBBF24));
            }
            else
            {
                btn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff151B26));
                btn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff94A3B8));
            }
        }

        dronePowerButton.setButtonText (droneOn ? "DRONE ACTIVE" : "DRONE MUTED");
        dronePowerButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff4FD1C5));
        prayerMacroButton.setButtonText (prayerOn ? "PRAYER SWELL: ACTIVE" : "PRAYER / TRANSITION");
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
    g.fillRect (0, 0, getWidth(), 62);
    g.setColour (juce::Colour (0xff1E2636));
    g.drawHorizontalLine (62, 0.0f, static_cast<float> (getWidth()));

    g.setFont (juce::Font (19.0f, juce::Font::bold));
    g.setColour (juce::Colour (0xffF8FAFC));
    g.drawText ("HALO ENGINE", 20, 10, 210, 24, juce::Justification::left);

    g.setFont (juce::Font (9.5f, juce::Font::bold));
    g.setColour (juce::Colour (0xff4FD1C5));
    g.drawText ("SERUM WAVETABLE + SUPERMASSIVE CLOUD WORKSTATION", 21, 34, 340, 16, juce::Justification::left);

    // Output Stereo Meters (Top Right)
    float lvlL = juce::jlimit (0.0f, 1.0f, audioProcessor.outLevelL.load (std::memory_order_relaxed) * 2.4f);
    float lvlR = juce::jlimit (0.0f, 1.0f, audioProcessor.outLevelR.load (std::memory_order_relaxed) * 2.4f);

    int meterX = getWidth() - 102;
    g.setColour (juce::Colour (0xff18202F));
    g.fillRoundedRectangle (static_cast<float> (meterX), 18.0f, 82.0f, 7.0f, 3.0f);
    g.fillRoundedRectangle (static_cast<float> (meterX), 29.0f, 82.0f, 7.0f, 3.0f);

    g.setColour (juce::Colour (0xff4FD1C5));
    g.fillRoundedRectangle (static_cast<float> (meterX), 18.0f, 82.0f * lvlL, 7.0f, 3.0f);
    g.fillRoundedRectangle (static_cast<float> (meterX), 29.0f, 82.0f * lvlR, 7.0f, 3.0f);

    // Smart Key Assist Banner
    int activeKey = static_cast<int> (*audioProcessor.apvts.getRawParameterValue ("DRONE_KEY"));
    bool assistOn = *audioProcessor.apvts.getRawParameterValue ("KEY_DETECT_ON") > 0.5f;
    bool showBanner = assistOn && (detectedKeyCache >= 0 && detectedKeyCache < 12 && detectedKeyCache != activeKey);

    if (showBanner)
    {
        const std::array<juce::String, 12> noteNames { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        juce::Rectangle<float> bannerRect (20.0f, 70.0f, static_cast<float> (getWidth() - 40), 28.0f);
        g.setColour (juce::Colour (0xffF59E0B).withAlpha (0.15f));
        g.fillRoundedRectangle (bannerRect, 6.0f);
        g.setColour (juce::Colour (0xffF59E0B).withAlpha (0.65f));
        g.drawRoundedRectangle (bannerRect, 6.0f, 1.0f);

        g.setFont (juce::Font (11.5f, juce::Font::bold));
        g.setColour (juce::Colour (0xffFBBF24));
        g.drawText ("SMART KEY ASSIST: BAND DETECTED IN KEY OF [ " + noteNames[static_cast<size_t> (detectedKeyCache)] + " ]",
                    32, 70, 500, 28, juce::Justification::left);
    }

    int topOffset = showBanner ? 106 : 74;

    if (currentTab == 0)
    {
        // ======================================================================
        // TAB 0: STAGE VIEW CARDS
        // ======================================================================
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (20.0f, static_cast<float> (topOffset), 280.0f, static_cast<float> (getHeight() - topOffset - 18), 10.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (20.0f, static_cast<float> (topOffset), 280.0f, static_cast<float> (getHeight() - topOffset - 18), 10.0f, 1.0f);

        g.setFont (juce::Font (11.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xff94A3B8));
        g.drawText ("TONAL CENTER / DRONE KEY", 34, topOffset + 12, 240, 18, juce::Justification::left);

        g.setFont (juce::Font (11.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xff94A3B8));
        g.drawText ("2D TIMBRE & WT MORPH PAD", 316, topOffset + 2, 280, 18, juce::Justification::left);

        // Right Column: Supermassive Cloud & Ambience Rack Card
        int rightX = getWidth() - 320;
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (static_cast<float> (rightX), static_cast<float> (topOffset), 300.0f, static_cast<float> (getHeight() - topOffset - 18), 10.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (static_cast<float> (rightX), static_cast<float> (topOffset), 300.0f, static_cast<float> (getHeight() - topOffset - 18), 10.0f, 1.0f);

        g.setFont (juce::Font (11.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xffA78BFA));
        g.drawText ("VALHALLA SUPERMASSIVE CLOUD ENGINE", rightX + 16, topOffset + 12, 270, 18, juce::Justification::left);

        // Band Auto-Duck Gain Reduction Meter
        float duckVal = audioProcessor.duckMeter.load (std::memory_order_relaxed);
        int duckBarY = getHeight() - 56;
        g.setFont (juce::Font (9.5f, juce::Font::bold));
        g.setColour (juce::Colour (0xffF59E0B));
        g.drawText ("BAND REACTIVE DUCKING", rightX + 18, duckBarY - 16, 200, 14, juce::Justification::left);

        g.setColour (juce::Colour (0xff18202F));
        g.fillRoundedRectangle (static_cast<float> (rightX + 18), static_cast<float> (duckBarY), 264.0f, 8.0f, 4.0f);
        g.setColour (juce::Colour (0xffF59E0B));
        g.fillRoundedRectangle (static_cast<float> (rightX + 18), static_cast<float> (duckBarY), 264.0f * duckVal, 8.0f, 4.0f);
    }
    else
    {
        // ======================================================================
        // TAB 1: SERUM / VITAL / SUPERMASSIVE SYNTH WORKSTATION CARDS
        // ======================================================================
        int cardTop = topOffset;
        int row1H = 198;
        int row2Top = cardTop + row1H + 10;
        int row2H = 152;
        int row3Top = row2Top + row2H + 10;
        int row3H = 128;

        // OSC A Card
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (20.0f, static_cast<float> (cardTop), 494.0f, static_cast<float> (row1H), 10.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (20.0f, static_cast<float> (cardTop), 494.0f, static_cast<float> (row1H), 10.0f, 1.0f);
        g.setFont (juce::Font (11.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xff4FD1C5));
        g.drawText ("OSC A // WAVETABLE & SPECTRAL WARP", 34, cardTop + 8, 280, 16, juce::Justification::left);

        // OSC B + Organic Air Card
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (526.0f, static_cast<float> (cardTop), 494.0f, static_cast<float> (row1H), 10.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (526.0f, static_cast<float> (cardTop), 494.0f, static_cast<float> (row1H), 10.0f, 1.0f);
        g.setFont (juce::Font (11.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xffA78BFA));
        g.drawText ("OSC B // HARMONIC LAYER & ORGANIC AIR", 540, cardTop + 8, 300, 16, juce::Justification::left);

        // Row 2 Left: ZDF Multi-Mode Filter + Drive
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (20.0f, static_cast<float> (row2Top), 494.0f, static_cast<float> (row2H), 10.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (20.0f, static_cast<float> (row2Top), 494.0f, static_cast<float> (row2H), 10.0f, 1.0f);
        g.setFont (juce::Font (11.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xff38BDF8));
        g.drawText ("ZDF ANALOG MULTI-MODE FILTER & SATURATION", 34, row2Top + 8, 320, 16, juce::Justification::left);

        // Row 2 Right: Dual LFO Modulation Matrix
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (526.0f, static_cast<float> (row2Top), 494.0f, static_cast<float> (row2H), 10.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (526.0f, static_cast<float> (row2Top), 494.0f, static_cast<float> (row2H), 10.0f, 1.0f);
        g.setFont (juce::Font (11.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xffF59E0B));
        g.drawText ("MODULATION MATRIX // LFO 1 (WT SCAN) & LFO 2 (CUTOFF)", 540, row2Top + 8, 360, 16, juce::Justification::left);

        // Row 3 Left: 6-Voice Harmonic Drone Mixer
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (20.0f, static_cast<float> (row3Top), 320.0f, static_cast<float> (row3H), 10.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (20.0f, static_cast<float> (row3Top), 320.0f, static_cast<float> (row3H), 10.0f, 1.0f);
        g.setFont (juce::Font (10.5f, juce::Font::bold));
        g.setColour (juce::Colour (0xff4FD1C5));
        g.drawText ("6-VOICE DRONE HARMONIC MIXER", 32, row3Top + 6, 260, 14, juce::Justification::left);

        // Row 3 Middle: Global Unison, Width, Glide & ADSR
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (350.0f, static_cast<float> (row3Top), 410.0f, static_cast<float> (row3H), 10.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (350.0f, static_cast<float> (row3Top), 410.0f, static_cast<float> (row3H), 10.0f, 1.0f);
        g.setFont (juce::Font (10.5f, juce::Font::bold));
        g.setColour (juce::Colour (0xffA78BFA));
        g.drawText ("7-VOICE UNISON, GLIDE & ENVELOPE", 362, row3Top + 6, 300, 14, juce::Justification::left);

        // Row 3 Right: Real-Time Stereo Oscilloscope
        juce::Rectangle<float> scopeRect (770.0f, static_cast<float> (row3Top), 250.0f, static_cast<float> (row3H));
        g.setColour (juce::Colour (0xff101622));
        g.fillRoundedRectangle (scopeRect, 10.0f);
        g.setColour (juce::Colour (0xff1E2636));
        g.drawRoundedRectangle (scopeRect, 10.0f, 1.0f);

        g.setFont (juce::Font (10.5f, juce::Font::bold));
        g.setColour (juce::Colour (0xff4FD1C5));
        g.drawText ("MASTER SCOPE", 782, row3Top + 6, 150, 14, juce::Justification::left);

        juce::Path scopePath;
        float midY = scopeRect.getCentreY() + 8.0f;
        float ampY = scopeRect.getHeight() * 0.34f;
        float startX = scopeRect.getX() + 12.0f;
        float stepX  = (scopeRect.getWidth() - 24.0f) / static_cast<float> (HaloEngineAudioProcessor::scopeSize - 1);

        for (int i = 0; i < HaloEngineAudioProcessor::scopeSize; ++i)
        {
            float sample = audioProcessor.scopeSamples[static_cast<size_t> (i)].load (std::memory_order_relaxed);
            float px = startX + i * stepX;
            float py = midY - sample * ampY;
            if (i == 0) scopePath.startNewSubPath (px, py);
            else scopePath.lineTo (px, py);
        }

        g.setColour (juce::Colour (0xff4FD1C5).withAlpha (0.28f));
        g.strokePath (scopePath, juce::PathStrokeType (4.0f));
        g.setColour (juce::Colour (0xff4FD1C5));
        g.strokePath (scopePath, juce::PathStrokeType (1.8f));
    }
}

void PluginEditor::resized()
{
    // Top Header Controls
    stageTabButton.setBounds (368, 15, 92, 30);
    synthTabButton.setBounds (466, 15, 118, 30);

    prevPresetButton.setBounds (596, 15, 26, 30);
    presetSelector.setBounds   (624, 15, 174, 30);
    nextPresetButton.setBounds (800, 15, 26, 30);

    keyAssistToggle.setBounds  (834, 15, 96, 30);
    applyKeyButton.setBounds   (getWidth() - 140, 72, 110, 24);

    int activeKey = static_cast<int> (*audioProcessor.apvts.getRawParameterValue ("DRONE_KEY"));
    bool assistOn = *audioProcessor.apvts.getRawParameterValue ("KEY_DETECT_ON") > 0.5f;
    bool showBanner = assistOn && (detectedKeyCache >= 0 && detectedKeyCache < 12 && detectedKeyCache != activeKey);
    int topOffset = showBanner ? 106 : 74;

    // ==========================================================================
    // TAB 0: STAGE VIEW LAYOUT
    // ==========================================================================
    int keyGridY = topOffset + 38;
    int btnW = 76;
    int btnH = 44;
    for (int i = 0; i < 12; ++i)
    {
        int col = i % 3;
        int row = i / 3;
        keyButtons[static_cast<size_t> (i)].setBounds (34 + col * (btnW + 10), keyGridY + row * (btnH + 10), btnW, btnH);
    }

    int droneBtnY = keyGridY + 4 * (btnH + 10) + 12;
    dronePowerButton.setBounds (34, droneBtnY, 248, 38);
    prayerMacroButton.setBounds (34, droneBtnY + 48, 248, 44);

    int rightX = getWidth() - 320;
    int padW = rightX - 316 - 16;
    morphPad.setBounds (316, topOffset + 22, padW, getHeight() - topOffset - 40);

    // Right Column: 3x3 Supermassive + Ambience Knob Grid
    int kW = 84;
    int kH = 74;
    int r1Y = topOffset + 42;
    int r2Y = r1Y + kH + 22;
    int r3Y = r2Y + kH + 22;

    auto placeKnob = [] (juce::Slider& s, juce::Label& l, int x, int y, int w, int h)
    {
        l.setBounds (x, y, w, 14);
        s.setBounds (x, y + 14, w, h);
    };

    placeKnob (droneLevelSlider, droneLevelLabel, rightX + 16,  r1Y, kW, kH);
    placeKnob (shimmerSlider,    shimmerLabel,    rightX + 108, r1Y, kW, kH);
    placeKnob (decaySlider,      decayLabel,      rightX + 200, r1Y, kW, kH);

    placeKnob (spaceWarpSlider,  spaceWarpLabel,  rightX + 16,  r2Y, kW, kH);
    placeKnob (spaceModSlider,   spaceModLabel,   rightX + 108, r2Y, kW, kH);
    placeKnob (spaceDampSlider,  spaceDampLabel,  rightX + 200, r2Y, kW, kH);

    placeKnob (autoDuckSlider,   autoDuckLabel,   rightX + 48,  r3Y, kW, kH);
    placeKnob (mixSlider,        mixLabel,        rightX + 168, r3Y, kW, kH);

    // ==========================================================================
    // TAB 1: SERUM / VITAL / SUPERMASSIVE SYNTH ENGINE LAYOUT
    // ==========================================================================
    int cardTop = topOffset;
    int row1H = 198;
    int row2Top = cardTop + row1H + 10;
    int row2H = 152;
    int row3Top = row2Top + row2H + 10;
    int row3H = 128;

    // OSC A Card
    oscATableBox.setBounds    (34,  cardTop + 30, 210, 26);
    oscAWarpModeBox.setBounds (34,  cardTop + 62, 210, 26);
    oscAPreview.setBounds     (34,  cardTop + 96, 210, 88);

    placeKnob (oscAPosSlider,   oscAPosLabel,   254, cardTop + 46, 78, 86);
    placeKnob (oscAWarpSlider,  oscAWarpLabel,  338, cardTop + 46, 78, 86);
    placeKnob (oscALevelSlider, oscALevelLabel, 422, cardTop + 46, 78, 86);

    // OSC B + Organic Air Card
    oscBTableBox.setBounds (540, cardTop + 30, 210, 26);
    oscBPreview.setBounds  (540, cardTop + 66, 210, 118);

    placeKnob (oscBPosSlider,   oscBPosLabel,   758, cardTop + 34, 62, 68);
    placeKnob (oscBOctSlider,   oscBOctLabel,   824, cardTop + 34, 62, 68);
    placeKnob (oscBLevelSlider, oscBLevelLabel, 890, cardTop + 34, 62, 68);
    placeKnob (airNoiseSlider,  airNoiseLabel,  952, cardTop + 34, 62, 68);

    // Row 2 Left: ZDF Filter + Drive
    filterModeBox.setBounds (34, row2Top + 32, 175, 28);
    placeKnob (filterResSlider, filterResLabel, 224, row2Top + 32, 86, 86);
    placeKnob (filterEnvSlider, filterEnvLabel, 318, row2Top + 32, 86, 86);
    placeKnob (driveSlider,     driveLabel,     412, row2Top + 32, 86, 86);

    // Row 2 Right: LFO 1 & LFO 2
    lfo1ShapeBox.setBounds (540, row2Top + 32, 115, 26);
    placeKnob (lfo1RateSlider, lfo1RateLabel, 662, row2Top + 30, 56, 68);
    placeKnob (lfo1WtSlider,   lfo1WtLabel,   720, row2Top + 30, 56, 68);

    lfo2ShapeBox.setBounds (784, row2Top + 32, 115, 26);
    placeKnob (lfo2RateSlider, lfo2RateLabel, 904, row2Top + 30, 56, 68);
    placeKnob (lfo2CutSlider,  lfo2CutLabel,  960, row2Top + 30, 56, 68);

    // Row 3 Left: 6-Voice Harmonic Drone Mixer Faders
    for (size_t i = 0; i < 6; ++i)
    {
        int fx = 32 + static_cast<int> (i) * 48;
        voiceLabels[i].setBounds (fx, row3Top + 24, 42, 12);
        voiceFaders[i].setBounds (fx + 7, row3Top + 38, 28, row3H - 44);
    }

    // Row 3 Middle: Global Unison, Width, Glide & ADSR
    int gStartX = 360;
    int gW = 76;
    placeKnob (detuneSlider,  detuneLabel,  gStartX + gW * 0, row3Top + 26, gW, 74);
    placeKnob (widthSlider,   widthLabel,   gStartX + gW * 1, row3Top + 26, gW, 74);
    placeKnob (glideSlider,   glideLabel,   gStartX + gW * 2, row3Top + 26, gW, 74);
    placeKnob (attackSlider,  attackLabel,  gStartX + gW * 3, row3Top + 26, gW, 74);
    placeKnob (releaseSlider, releaseLabel, gStartX + gW * 4, row3Top + 26, gW, 74);

    // Playable 2-Octave Keyboard along bottom
    int kbY = row3Top + row3H + 10;
    midiKeyboard.setBounds (20, kbY, getWidth() - 40, getHeight() - kbY - 12);
}
