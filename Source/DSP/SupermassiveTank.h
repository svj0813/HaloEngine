#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <cmath>

// ==============================================================================
// HALO ENGINE - VALHALLA SUPERMASSIVE-INSPIRED 8-CHANNEL MODULATED FDN TANK
// Pre-Delay -> 4-Stage Allpass Cloud Diffuser -> 8-Tap Hadamard Modulated FDN
// -> In-Loop Dual-Pitch Shimmer (+12 Oct & +19 Fifth) + Analog Tape Saturation
// ==============================================================================
class SupermassiveTank
{
public:
    SupermassiveTank() = default;

    void prepare (double newSampleRate)
    {
        sampleRate = newSampleRate;
        const double srScale = sampleRate / 44100.0;

        // Pre-delay buffer (up to 150 ms)
        int preDelaySize = static_cast<int> (sampleRate * 0.15) + 8;
        preDelayL.assign (static_cast<size_t> (preDelaySize), 0.0f);
        preDelayR.assign (static_cast<size_t> (preDelaySize), 0.0f);
        preWriteIdx = 0;

        // 4-Stage Cascaded Allpass Diffusers (coprime lengths for zero metallic flutter)
        static constexpr std::array<int, 4> apBaseL { 241, 419, 683, 1031 };
        static constexpr std::array<int, 4> apBaseR { 263, 449, 719, 1069 };

        for (int i = 0; i < 4; ++i)
        {
            int lenL = static_cast<int> (apBaseL[static_cast<size_t> (i)] * srScale) + 4;
            int lenR = static_cast<int> (apBaseR[static_cast<size_t> (i)] * srScale) + 4;
            allpassL[static_cast<size_t> (i)].buf.assign (static_cast<size_t> (lenL), 0.0f);
            allpassR[static_cast<size_t> (i)].buf.assign (static_cast<size_t> (lenR), 0.0f);
            allpassL[static_cast<size_t> (i)].idx = 0;
            allpassR[static_cast<size_t> (i)].idx = 0;
        }

        // 8-Channel Coprime Modulated FDN Delay Lines (29 ms to 134 ms)
        static constexpr std::array<double, numLines> fdnMs {
            29.3, 37.9, 47.7, 59.1, 73.3, 89.7, 109.3, 133.9
        };

        for (int c = 0; c < numLines; ++c)
        {
            baseDelaySamples[static_cast<size_t> (c)] = static_cast<float> ((fdnMs[static_cast<size_t> (c)] * 0.001) * sampleRate);
            int maxLen = static_cast<int> (baseDelaySamples[static_cast<size_t> (c)] * 1.45f) + 64;
            fdnBuffers[static_cast<size_t> (c)].assign (static_cast<size_t> (maxLen), 0.0f);
            fdnWriteIdx[static_cast<size_t> (c)] = 0;
            lfoPhases[static_cast<size_t> (c)] = static_cast<double> (c) / static_cast<double> (numLines);
            dampState[static_cast<size_t> (c)] = 0.0f;
            hpState[static_cast<size_t> (c)] = 0.0f;
        }

        // Dual-Head Granular Shimmer Buffers (Octave +12 & Fifth+Octave +19)
        int shimLen = static_cast<int> (sampleRate * 0.11);
        shimBufL.assign (static_cast<size_t> (shimLen), 0.0f);
        shimBufR.assign (static_cast<size_t> (shimLen), 0.0f);
        shimWriteIdx = 0;
        shimPhaseOct = 0.0f;
        shimPhaseFifth = 0.0f;
    }

    void reset()
    {
        std::fill (preDelayL.begin(), preDelayL.end(), 0.0f);
        std::fill (preDelayR.begin(), preDelayR.end(), 0.0f);
        for (int i = 0; i < 4; ++i)
        {
            std::fill (allpassL[static_cast<size_t> (i)].buf.begin(), allpassL[static_cast<size_t> (i)].buf.end(), 0.0f);
            std::fill (allpassR[static_cast<size_t> (i)].buf.begin(), allpassR[static_cast<size_t> (i)].buf.end(), 0.0f);
        }
        for (int c = 0; c < numLines; ++c)
        {
            std::fill (fdnBuffers[static_cast<size_t> (c)].begin(), fdnBuffers[static_cast<size_t> (c)].end(), 0.0f);
            dampState[static_cast<size_t> (c)] = 0.0f;
            hpState[static_cast<size_t> (c)] = 0.0f;
        }
        std::fill (shimBufL.begin(), shimBufL.end(), 0.0f);
        std::fill (shimBufR.begin(), shimBufR.end(), 0.0f);
    }

    // Parameters:
    // decay:     0.0 -> 1.0 (feedback energy, up to infinite cloud sustain)
    // warp:      0.0 -> 1.0 (allpass diffusion & delay density: tight echo -> huge cloud bloom)
    // modDepth:  0.0 -> 1.0 (chorus/ensemble pitch drift inside FDN tank)
    // shimmer:   0.0 -> 1.0 (in-loop +12 & +19 octave shimmer injection)
    // highDamp:  0.0 -> 1.0 (air absorption high-frequency damping)
    // width:     0.0 -> 1.0 (stereo spread)
    void processSample (float inL, float inR, float& outL, float& outR,
                        float decay, float warp, float modDepth,
                        float shimmer, float highDamp, float width) noexcept
    {
        const int preLen = static_cast<int> (preDelayL.size());
        preDelayL[static_cast<size_t> (preWriteIdx)] = inL;
        preDelayR[static_cast<size_t> (preWriteIdx)] = inR;

        // Warp scales pre-delay subtly (12ms -> 55ms) for grand cathedral depth
        float preDelaySamples = (0.012f + warp * 0.043f) * static_cast<float> (sampleRate);
        int preReadIdx = (preWriteIdx - static_cast<int> (preDelaySamples) + preLen) % preLen;
        float diffL = preDelayL[static_cast<size_t> (preReadIdx)];
        float diffR = preDelayR[static_cast<size_t> (preReadIdx)];
        preWriteIdx = (preWriteIdx + 1) % preLen;

        // 1. 4-Stage Allpass Diffuser Network (Supermassive cloud smearing)
        float apCoeff = 0.42f + warp * 0.30f; // 0.42 (clear) -> 0.72 (dense velvet cloud)
        for (int i = 0; i < 4; ++i)
        {
            diffL = processAllpass (allpassL[static_cast<size_t> (i)], diffL, apCoeff);
            diffR = processAllpass (allpassR[static_cast<size_t> (i)], diffR, apCoeff);
        }

        // 2. Dual-Pitch Granular Shimmer (+12 Octave & +19 Fifth+Octave)
        const int shimLen = static_cast<int> (shimBufL.size());
        shimBufL[static_cast<size_t> (shimWriteIdx)] = diffL + lastFeedbackL * 0.32f;
        shimBufR[static_cast<size_t> (shimWriteIdx)] = diffR + lastFeedbackR * 0.32f;

        float octL = readPitchShifter (shimBufL, shimWriteIdx, shimPhaseOct);
        float octR = readPitchShifter (shimBufR, shimWriteIdx, shimPhaseOct);
        float fifthL = readPitchShifter (shimBufL, shimWriteIdx, shimPhaseFifth);
        float fifthR = readPitchShifter (shimBufR, shimWriteIdx, shimPhaseFifth);

        shimWriteIdx = (shimWriteIdx + 1) % shimLen;
        shimPhaseOct += 1.0f / static_cast<float> (shimLen);
        if (shimPhaseOct >= 1.0f) shimPhaseOct -= 1.0f;

        // +19 semitones (octave + fifth = 2.9966x speed -> +1.9966x read advance)
        shimPhaseFifth += 1.9966f / static_cast<float> (shimLen);
        if (shimPhaseFifth >= 1.0f) shimPhaseFifth -= 1.0f;

        float shimSigL = (octL * 0.72f + fifthL * 0.28f) * shimmer * 0.65f;
        float shimSigR = (octR * 0.72f + fifthR * 0.28f) * shimmer * 0.65f;

        // 3. Read 8 Modulated FDN Delay Lines with Linear Interpolation
        std::array<float, numLines> lineOut {};
        const double lfoRateBase = 0.28 / sampleRate;
        const float modExcursion = (1.5f + modDepth * 26.0f) * static_cast<float> (sampleRate / 44100.0);
        const float sizeScale    = 0.78f + warp * 0.48f;

        for (int c = 0; c < numLines; ++c)
        {
            lfoPhases[static_cast<size_t> (c)] += lfoRateBase * (1.0 + 0.17 * c);
            if (lfoPhases[static_cast<size_t> (c)] >= 1.0)
                lfoPhases[static_cast<size_t> (c)] -= 1.0;

            float lfoVal = static_cast<float> (std::sin (lfoPhases[static_cast<size_t> (c)] * juce::MathConstants<double>::twoPi));
            float delaySamps = baseDelaySamples[static_cast<size_t> (c)] * sizeScale + lfoVal * modExcursion;

            auto& buf = fdnBuffers[static_cast<size_t> (c)];
            const int bSize = static_cast<int> (buf.size());
            float readPos = static_cast<float> (fdnWriteIdx[static_cast<size_t> (c)]) - delaySamps;
            while (readPos < 0.0f) readPos += static_cast<float> (bSize);

            int i0 = static_cast<int> (readPos) % bSize;
            int i1 = (i0 + 1) % bSize;
            float frac = readPos - std::floor (readPos);
            lineOut[static_cast<size_t> (c)] = buf[static_cast<size_t> (i0)] * (1.0f - frac)
                                             + buf[static_cast<size_t> (i1)] * frac;
        }

        // 4. Orthogonal 8x8 Hadamard Mixing Matrix (Unitary Energy-Preserving Diffusion)
        // Fast Walsh-Hadamard Transform (O(N log N), zero gain buildup, maximum echo density)
        std::array<float, numLines> mixed = lineOut;
        for (int step = 1; step < numLines; step <<= 1)
        {
            for (int i = 0; i < numLines; i += (step << 1))
            {
                for (int j = 0; j < step; ++j)
                {
                    float a = mixed[static_cast<size_t> (i + j)];
                    float b = mixed[static_cast<size_t> (i + j + step)];
                    mixed[static_cast<size_t> (i + j)]        = a + b;
                    mixed[static_cast<size_t> (i + j + step)] = a - b;
                }
            }
        }
        // Normalize by 1 / sqrt(8) = 0.35355339f
        for (int c = 0; c < numLines; ++c)
            mixed[static_cast<size_t> (c)] *= 0.35355339f;

        // 5. Feedback Damping, Low-Cut Mud Removal, Analog Tape Saturation & Write Back
        // Decay curve mapped smoothly from medium hall (0.72) up to near-infinite bloom (0.985)
        float fbGain = 0.68f + std::pow (juce::jlimit (0.0f, 1.0f, decay), 0.65f) * 0.305f;
        float dampCoeff = juce::jlimit (0.08f, 0.88f, 0.16f + highDamp * 0.68f);

        float sumL = 0.0f;
        float sumR = 0.0f;

        for (int c = 0; c < numLines; ++c)
        {
            // One-pole lowpass air absorption inside feedback loop
            float x = mixed[static_cast<size_t> (c)] * fbGain;
            dampState[static_cast<size_t> (c)] += dampCoeff * (x - dampState[static_cast<size_t> (c)]);
            float damped = dampState[static_cast<size_t> (c)];

            // Gentle 65Hz highpass inside feedback loop so infinite tails never turn muddy
            hpState[static_cast<size_t> (c)] += 0.009f * (damped - hpState[static_cast<size_t> (c)]);
            float cleanTail = damped - hpState[static_cast<size_t> (c)];

            // Warm analog tape soft-clipper inside loop
            float saturated = std::tanh (cleanTail * 1.15f) / 1.15f;

            // Inject stereo input + shimmer into alternating FDN channels
            float inject = (c % 2 == 0) ? (diffL * 0.35f + shimSigL * 0.42f)
                                        : (diffR * 0.35f + shimSigR * 0.42f);

            auto& buf = fdnBuffers[static_cast<size_t> (c)];
            const int bSize = static_cast<int> (buf.size());
            buf[static_cast<size_t> (fdnWriteIdx[static_cast<size_t> (c)])] = saturated + inject;
            fdnWriteIdx[static_cast<size_t> (c)] = (fdnWriteIdx[static_cast<size_t> (c)] + 1) % bSize;

            if (c % 2 == 0) sumL += lineOut[static_cast<size_t> (c)];
            else            sumR += lineOut[static_cast<size_t> (c)];
        }

        sumL *= 0.45f;
        sumR *= 0.45f;

        lastFeedbackL = sumL;
        lastFeedbackR = sumR;

        // Mid/Side Stereo Width Enhancement
        float mid  = 0.5f * (sumL + sumR);
        float side = 0.5f * (sumL - sumR) * (0.4f + width * 0.95f);
        outL = mid + side;
        outR = mid - side;
    }

private:
    struct AllpassStage
    {
        std::vector<float> buf;
        int idx = 0;
    };

    static inline float processAllpass (AllpassStage& ap, float input, float coeff) noexcept
    {
        const int len = static_cast<int> (ap.buf.size());
        float bufOut = ap.buf[static_cast<size_t> (ap.idx)];
        float out = -input * coeff + bufOut;
        ap.buf[static_cast<size_t> (ap.idx)] = input + bufOut * coeff;
        ap.idx = (ap.idx + 1) % len;
        return out;
    }

    static inline float readPitchShifter (const std::vector<float>& buf, int writeIdx, float phase) noexcept
    {
        const int len = static_cast<int> (buf.size());
        float phaseA = phase;
        float phaseB = std::fmod (phase + 0.5f, 1.0f);

        float offsetA = (1.0f - phaseA) * static_cast<float> (len - 2);
        float offsetB = (1.0f - phaseB) * static_cast<float> (len - 2);

        int idxA = (writeIdx - static_cast<int> (offsetA) + len) % len;
        int idxB = (writeIdx - static_cast<int> (offsetB) + len) % len;

        float winA = std::sin (phaseA * juce::MathConstants<float>::pi);
        float winB = std::sin (phaseB * juce::MathConstants<float>::pi);

        return buf[static_cast<size_t> (idxA)] * winA + buf[static_cast<size_t> (idxB)] * winB;
    }

    static constexpr int numLines = 8;
    double sampleRate = 44100.0;

    std::vector<float> preDelayL, preDelayR;
    int preWriteIdx = 0;

    std::array<AllpassStage, 4> allpassL, allpassR;

    std::array<std::vector<float>, numLines> fdnBuffers;
    std::array<int, numLines> fdnWriteIdx {};
    std::array<float, numLines> baseDelaySamples {};
    std::array<double, numLines> lfoPhases {};
    std::array<float, numLines> dampState {};
    std::array<float, numLines> hpState {};

    std::vector<float> shimBufL, shimBufR;
    int shimWriteIdx = 0;
    float shimPhaseOct = 0.0f;
    float shimPhaseFifth = 0.0f;

    float lastFeedbackL = 0.0f;
    float lastFeedbackR = 0.0f;
};
