#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <random>

// ==============================================================================
// HALO ENGINE - PHASE 3: MODULATION ENGINE (DUAL ADSR + DUAL LFOs) & ZDF FILTER
// ==============================================================================
class HaloLFO
{
public:
    HaloLFO (uint32_t seed = 99) : rng (seed), dist (-1.0f, 1.0f) {}

    void prepare (double newSampleRate)
    {
        sampleRate = newSampleRate;
        phase = 0.0;
        currentRandom = dist (rng);
        nextRandom = dist (rng);
    }

    // shape: 0 = Sine, 1 = Triangle, 2 = Smooth Wander (Interpolated Random)
    inline float process (float rateHz, int shape) noexcept
    {
        double dt = static_cast<double> (juce::jlimit (0.02f, 20.0f, rateHz)) / sampleRate;
        phase += dt;
        if (phase >= 1.0)
        {
            phase -= 1.0;
            currentRandom = nextRandom;
            nextRandom = dist (rng);
        }

        if (shape == 0)
            return static_cast<float> (std::sin (phase * juce::MathConstants<double>::twoPi));

        if (shape == 1)
        {
            float p = static_cast<float> (phase);
            return (p < 0.5f) ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);
        }

        // Smooth cosine-interpolated random wander (Serum/Omnisphere organic drift)
        float t = static_cast<float> (phase);
        float smoothT = t * t * (3.0f - 2.0f * t);
        return currentRandom + (nextRandom - currentRandom) * smoothT;
    }

private:
    double sampleRate = 44100.0;
    double phase = 0.0;
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
    float currentRandom = 0.0f;
    float nextRandom = 0.0f;
};

class HaloADSR
{
public:
    enum class Stage { Idle, Attack, Decay, Sustain, Release };

    void prepare (double newSampleRate)
    {
        sampleRate = newSampleRate;
        reset();
    }

    void reset()
    {
        stage = Stage::Idle;
        value = 0.0f;
    }

    void noteOn()
    {
        stage = Stage::Attack;
    }

    void noteOff()
    {
        if (stage != Stage::Idle)
            stage = Stage::Release;
    }

    bool isActive() const noexcept { return stage != Stage::Idle; }
    float getValue() const noexcept { return value; }

    inline float process (float attackSec, float decaySec, float sustainLvl, float releaseSec) noexcept
    {
        const float sr = static_cast<float> (sampleRate);

        if (stage == Stage::Idle)
        {
            value = 0.0f;
            return 0.0f;
        }

        if (stage == Stage::Attack)
        {
            float rate = 1.0f / (sr * juce::jmax (0.004f, attackSec));
            value += rate;
            if (value >= 1.0f)
            {
                value = 1.0f;
                stage = Stage::Decay;
            }
        }
        else if (stage == Stage::Decay)
        {
            float target = juce::jlimit (0.0f, 1.0f, sustainLvl);
            float coeff = std::exp (-1.0f / (sr * juce::jmax (0.01f, decaySec) * 0.35f));
            value = target + (value - target) * coeff;
            if (std::abs (value - target) < 0.0005f)
            {
                value = target;
                stage = Stage::Sustain;
            }
        }
        else if (stage == Stage::Sustain)
        {
            value = juce::jlimit (0.0f, 1.0f, sustainLvl);
        }
        else if (stage == Stage::Release)
        {
            float coeff = std::exp (-1.0f / (sr * juce::jmax (0.015f, releaseSec) * 0.35f));
            value *= coeff;
            if (value < 0.0002f)
            {
                value = 0.0f;
                stage = Stage::Idle;
            }
        }

        // Smooth S-curve shaping
        return value * value * (3.0f - 2.0f * value);
    }

private:
    double sampleRate = 44100.0;
    Stage stage = Stage::Idle;
    float value = 0.0f;
};

// ==============================================================================
// ZERO-DELAY-FEEDBACK (ZDF) 4-MODE STEREO ANALOG FILTER
// 0: LP 24dB (Ladder), 1: LP 12dB (OB-6), 2: Bandpass Formant, 3: Notch/Comb
// ==============================================================================
class HaloMultiFilter
{
public:
    void prepare (double sampleRate)
    {
        juce::dsp::ProcessSpec spec { sampleRate, 512, 1 };
        for (auto* f : { &svfL1, &svfL2, &svfR1, &svfR2 })
            f->prepare (spec);
    }

    void reset()
    {
        for (auto* f : { &svfL1, &svfL2, &svfR1, &svfR2 })
            f->reset();
    }

    inline void processStereo (float& inOutL, float& inOutR, float cutoffHz, float resonanceNorm, int mode) noexcept
    {
        float freq = juce::jlimit (35.0f, 16500.0f, cutoffHz);
        float q    = 0.65f + juce::jlimit (0.0f, 1.0f, resonanceNorm) * 2.4f;

        if (mode == 0)
        {
            // Cascaded 24dB/oct Ladder-style Lowpass
            svfL1.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
            svfL2.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
            svfR1.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
            svfR2.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

            svfL1.setCutoffFrequency (freq); svfL2.setCutoffFrequency (freq);
            svfR1.setCutoffFrequency (freq); svfR2.setCutoffFrequency (freq);
            svfL1.setResonance (q);          svfL2.setResonance (q * 0.85f);
            svfR1.setResonance (q);          svfR2.setResonance (q * 0.85f);

            inOutL = svfL2.processSample (0, svfL1.processSample (0, inOutL));
            inOutR = svfR2.processSample (0, svfR1.processSample (0, inOutR));
        }
        else if (mode == 1)
        {
            // Open 12dB/oct Oberheim-style Lowpass
            svfL1.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
            svfR1.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
            svfL1.setCutoffFrequency (freq);
            svfR1.setCutoffFrequency (freq);
            svfL1.setResonance (q);
            svfR1.setResonance (q);

            inOutL = svfL1.processSample (0, inOutL);
            inOutR = svfR1.processSample (0, inOutR);
        }
        else if (mode == 2)
        {
            // Bandpass Vocal Formant
            svfL1.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
            svfR1.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
            svfL1.setCutoffFrequency (freq);
            svfR1.setCutoffFrequency (freq);
            svfL1.setResonance (q * 1.15f);
            svfR1.setResonance (q * 1.15f);

            inOutL = svfL1.processSample (0, inOutL) * 1.45f;
            inOutR = svfR1.processSample (0, inOutR) * 1.45f;
        }
        else
        {
            // True ZDF Harmonic Phaser Notch (Input minus Bandpass)
            svfL1.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
            svfR1.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
            svfL1.setCutoffFrequency (freq);
            svfR1.setCutoffFrequency (freq);
            svfL1.setResonance (q * 1.25f);
            svfR1.setResonance (q * 1.25f);

            inOutL = inOutL - svfL1.processSample (0, inOutL) * 0.90f;
            inOutR = inOutR - svfR1.processSample (0, inOutR) * 0.90f;
        }
    }

private:
    juce::dsp::StateVariableTPTFilter<float> svfL1, svfL2, svfR1, svfR2;
};
