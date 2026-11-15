#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>
#include <random>

class WavetableBank
{
public:
    static constexpr int numTables = 5;
    static constexpr int numFrames = 16;
    static constexpr int tableSize = 2048;

    WavetableBank()
    {
        buildAllTables();
    }

    inline float lookup (int tableIdx, float wtPos, double phase) const noexcept
    {
        const int t = juce::jlimit (0, numTables - 1, tableIdx);
        const float scaledFrame = juce::jlimit (0.0f, 1.0f, wtPos) * static_cast<float> (numFrames - 1);
        const int f0 = static_cast<int> (scaledFrame);
        const int f1 = juce::jmin (numFrames - 1, f0 + 1);
        const float fFrac = scaledFrame - static_cast<float> (f0);

        double wrapped = phase - std::floor (phase);
        double pos = wrapped * static_cast<double> (tableSize);
        int i0 = static_cast<int> (pos) & (tableSize - 1);
        int i1 = (i0 + 1) & (tableSize - 1);
        float pFrac = static_cast<float> (pos - std::floor (pos));

        const auto& frameA = tables[static_cast<size_t> (t)][static_cast<size_t> (f0)];
        const auto& frameB = tables[static_cast<size_t> (t)][static_cast<size_t> (f1)];

        float s0 = frameA[static_cast<size_t> (i0)] * (1.0f - pFrac) + frameA[static_cast<size_t> (i1)] * pFrac;
        float s1 = frameB[static_cast<size_t> (i0)] * (1.0f - pFrac) + frameB[static_cast<size_t> (i1)] * pFrac;

        return s0 * (1.0f - fFrac) + s1 * fFrac;
    }

    float getPreviewSample (int tableIdx, float wtPos, float normX) const noexcept
    {
        return lookup (tableIdx, wtPos, static_cast<double> (normX));
    }

private:
    void buildAllTables()
    {
        const double twoPi = juce::MathConstants<double>::twoPi;

        for (int t = 0; t < numTables; ++t)
        {
            for (int f = 0; f < numFrames; ++f)
            {
                float morph = static_cast<float> (f) / static_cast<float> (numFrames - 1);
                auto& frame = tables[static_cast<size_t> (t)][static_cast<size_t> (f)];
                frame.fill (0.0f);

                const int maxHarmonics = 48;
                for (int h = 1; h <= maxHarmonics; ++h)
                {
                    float amp = 0.0f;
                    double phaseOffset = 0.0;

                    if (t == 0)
                    {
                        float triAmp = (h % 2 == 1) ? (1.0f / static_cast<float> (h * h)) : 0.0f;
                        float sawAmp = 1.0f / static_cast<float> (h);
                        float sineAmp = (h == 1) ? 1.0f : 0.0f;

                        if (morph < 0.5f)
                        {
                            float m = morph * 2.0f;
                            amp = sineAmp * (1.0f - m) + triAmp * m;
                        }
                        else
                        {
                            float m = (morph - 0.5f) * 2.0f;
                            amp = triAmp * (1.0f - m) + sawAmp * m;
                        }
                        phaseOffset = (h % 2 == 1 && morph < 0.5f) ? 0.25 : 0.0;
                    }
                    else if (t == 1)
                    {
                        float f1 = 2.5f + morph * 4.5f;
                        float f2 = 6.0f + morph * 12.0f;
                        float fh = static_cast<float> (h);
                        float g1 = std::exp (-std::pow ((fh - f1) / 1.8f, 2.0f));
                        float g2 = 0.72f * std::exp (-std::pow ((fh - f2) / 2.6f, 2.0f));
                        amp = (1.0f / std::pow (fh, 0.75f)) * (0.28f + g1 * 1.4f + g2 * 1.2f);
                    }
                    else if (t == 2)
                    {
                        float fh = static_cast<float> (h);
                        bool isBellPartial = (h == 1 || h == 2 || h == 3 || h == 4 || h == 6 || h == 9 || h == 12 || h == 16);
                        float shimmerBoost = (h > 4) ? (0.2f + morph * 1.3f) : 1.0f;
                        amp = isBellPartial ? (shimmerBoost / std::pow (fh, 0.65f)) : (0.04f * morph / fh);
                    }
                    else if (t == 3)
                    {
                        float duty = 0.50f - morph * 0.38f;
                        amp = std::abs (std::sin (juce::MathConstants<float>::pi * h * duty)) / static_cast<float> (h);
                    }
                    else
                    {
                        float fh = static_cast<float> (h);
                        float bowBridgeNotch = std::abs (std::sin (fh * (0.35f + morph * 0.45f)));
                        amp = (1.0f / std::pow (fh, 0.85f)) * (0.35f + 0.85f * bowBridgeNotch);
                    }

                    float lanczos = std::sin (juce::MathConstants<float>::pi * h / (maxHarmonics + 1))
                                  / (juce::MathConstants<float>::pi * h / (maxHarmonics + 1));
                    amp *= lanczos;

                    for (int i = 0; i < tableSize; ++i)
                    {
                        double p = static_cast<double> (i) / static_cast<double> (tableSize);
                        frame[static_cast<size_t> (i)] += amp * static_cast<float> (std::sin (twoPi * (h * p + phaseOffset)));
                    }
                }

                float maxPeak = 0.001f;
                for (float s : frame)
                    maxPeak = juce::jmax (maxPeak, std::abs (s));

                float normScale = 0.95f / maxPeak;
                for (float& s : frame)
                    s *= normScale;
            }
        }
    }

    std::array<std::array<std::array<float, tableSize>, numFrames>, numTables> tables {};
};

class OrganicAirGenerator
{
public:
    OrganicAirGenerator() : rng (1337), dist (-1.0f, 1.0f) {}

    void prepare (double sampleRate)
    {
        juce::dsp::ProcessSpec spec { sampleRate, 512, 1 };
        bpFilterL.prepare (spec);
        bpFilterR.prepare (spec);
        bpFilterL.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
        bpFilterR.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
        bpFilterL.setCutoffFrequency (2800.0f);
        bpFilterR.setCutoffFrequency (3100.0f);
        bpFilterL.setResonance (0.85f);
        bpFilterR.setResonance (0.85f);
    }

    inline void processSample (float& outL, float& outR, float brightness, float amount) noexcept
    {
        if (amount <= 0.001f)
        {
            outL = 0.0f;
            outR = 0.0f;
            return;
        }

        float whiteL = dist (rng);
        float whiteR = dist (rng);

        b0L = 0.99765f * b0L + whiteL * 0.0990460f;
        b1L = 0.96300f * b1L + whiteL * 0.2965164f;
        b2L = 0.57000f * b2L + whiteL * 1.0526913f;
        float pinkL = (b0L + b1L + b2L + whiteL * 0.1848f) * 0.18f;

        b0R = 0.99765f * b0R + whiteR * 0.0990460f;
        b1R = 0.96300f * b1R + whiteR * 0.2965164f;
        b2R = 0.57000f * b2R + whiteR * 1.0526913f;
        float pinkR = (b0R + b1R + b2R + whiteR * 0.1848f) * 0.18f;

        float airFreq = 1400.0f + brightness * 4200.0f;
        bpFilterL.setCutoffFrequency (airFreq);
        bpFilterR.setCutoffFrequency (airFreq * 1.08f);

        outL = bpFilterL.processSample (0, pinkL) * amount * 0.35f;
        outR = bpFilterR.processSample (0, pinkR) * amount * 0.35f;
    }

private:
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
    float b0L = 0.0f, b1L = 0.0f, b2L = 0.0f;
    float b0R = 0.0f, b1R = 0.0f, b2R = 0.0f;
    juce::dsp::StateVariableTPTFilter<float> bpFilterL, bpFilterR;
};
