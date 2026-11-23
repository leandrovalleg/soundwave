// Tune — real-time pitch correction: YIN pitch detection + crossfaded dual-tap pitch shifter.
#include "Common.h"

struct Tune : SWEffect
{
    static inline const juce::StringArray keys { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
    static inline const juce::StringArray scales { "Chromatic","Major","Minor","Harmonic Minor","Pentatonic Major","Pentatonic Minor","Blues","Dorian" };
    static constexpr int scaleMasks[] = {
        0b111111111111, 0b101010110101, 0b010110101101, 0b100110101101,
        0b001010010101, 0b010010101001, 0b010011101001, 0b011010101101 }; // bit n = semitone n above key

    static Layout layout()
    {
        Layout l;
        l.add(choice("key", "Key", keys, 0));
        l.add(choice("scale", "Scale", scales, 1));
        l.add(param("speed", "Retune Speed", 0, 400, 20, "ms", 60));
        l.add(param("amount", "Amount", 0, 100, 100, "%"));
        l.add(choice("range", "Voice", { "Low", "Mid", "High" }, 1));
        l.add(param("mix", "Mix", 0, 100, 100, "%"));
        return l;
    }
    Tune() : SWEffect(layout()) {}

    static constexpr int Win = 2048, Hop = 256;

    void prepareToPlay(double sr, int) override
    {
        fs = sr;
        ring.assign(Win, 0.0f); ringPos = 0; hopCount = 0;
        dly.assign(8192, 0.0f); for (auto& d : dly2) d.assign(8192, 0.0f);
        wpos = 0; phase = 0; ratio = 1; targetRatio = 1;
        diff.assign(Win / 2, 0.0f);
    }

    // YIN: returns frequency in Hz or 0 when unvoiced.
    float detect(float minHz, float maxHz)
    {
        const int minLag = (int) (fs / maxHz), maxLag = juce::jmin((int) (fs / minHz), Win / 2 - 1);
        const int W = Win / 2;
        auto at = [&](int i) { return ring[(size_t) ((ringPos + i) % Win)]; };
        float run = 0; diff[0] = 1;
        for (int tau = 1; tau <= maxLag; ++tau)
        {
            float d = 0;
            for (int i = 0; i < W; ++i) { float x = at(i) - at(i + tau); d += x * x; }
            run += d;
            diff[(size_t) tau] = run > 0 ? d * tau / run : 1;     // cumulative mean normalised difference
        }
        for (int tau = minLag; tau < maxLag; ++tau)
            if (diff[(size_t) tau] < 0.15f)
            {
                while (tau + 1 < maxLag && diff[(size_t) tau + 1] < diff[(size_t) tau]) ++tau;
                float a = diff[(size_t) tau - 1], b = diff[(size_t) tau], c = diff[(size_t) tau + 1];
                float shift = (a - c) / (2 * (a - 2 * b + c) + 1e-9f);          // parabolic refinement
                return (float) (fs / (tau + juce::jlimit(-1.0f, 1.0f, shift)));
            }
        return 0;
    }

    float snap(float hz) const
    {
        const float midi = 69 + 12 * std::log2(hz / 440.0f);
        const int key = geti("key"), mask = scaleMasks[geti("scale")];
        float best = midi; float bestDist = 1e9f;
        for (int n = (int) std::floor(midi) - 6; n <= (int) std::ceil(midi) + 6; ++n)
            if (mask & (1 << (((n - key) % 12 + 12) % 12)))
                if (float d = std::abs(n - midi); d < bestDist) { bestDist = d; best = (float) n; }
        return 440.0f * std::pow(2.0f, (best - 69) / 12.0f);
    }

    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals nd;
        static const float lo[] = { 60, 90, 150 }, hi[] = { 400, 700, 1200 };
        const int r = geti("range");
        const float amount = get("amount") / 100.0f, mix = get("mix") / 100.0f;
        const float speedMs = get("speed");
        const float smooth = speedMs <= 0.5f ? 0.0f : std::exp(-1.0f / (0.001f * speedMs * (float) fs));
        const int chs = juce::jmin(2, buf.getNumChannels());
        const float shifterLen = (float) (0.03 * fs);                    // 30 ms grain

        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            float mono = 0; for (int c = 0; c < chs; ++c) mono += buf.getSample(c, i) / chs;
            ring[(size_t) ringPos] = mono; ringPos = (ringPos + 1) % Win;
            // ponytail: O(N^2) YIN every 256 samples (~5% of a core); switch to FFT autocorrelation if CPU matters
            if (++hopCount >= Hop)
            {
                hopCount = 0;
                float f = detect(lo[r], hi[r]);
                targetRatio = f > 0 ? std::pow(snap(f) / f, amount) : 1.0f;
            }
            ratio = smooth * ratio + (1 - smooth) * targetRatio;

            for (int c = 0; c < chs; ++c) dly2[c][(size_t) wpos] = buf.getSample(c, i);
            phase += (1.0f - ratio) / shifterLen;
            phase -= std::floor(phase);
            const float p2 = std::fmod(phase + 0.5f, 1.0f);
            const float g1 = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * phase);
            const float g2 = 1.0f - g1;
            for (int c = 0; c < chs; ++c)
            {
                auto tap = [&](float delay) {
                    float rp = (float) wpos - 1.0f - delay; while (rp < 0) rp += 8192;
                    int i0 = (int) rp; float fr = rp - i0;
                    return dly2[c][(size_t) i0] + fr * (dly2[c][(size_t) ((i0 + 1) & 8191)] - dly2[c][(size_t) i0]);
                };
                float wet = tap(phase * shifterLen) * g1 + tap(p2 * shifterLen) * g2;
                buf.setSample(c, i, buf.getSample(c, i) * (1 - mix) + wet * mix);
            }
            wpos = (wpos + 1) & 8191;
        }
    }

    double fs = 48000;
    std::vector<float> ring, diff, dly, dly2[2];
    int ringPos = 0, hopCount = 0, wpos = 0;
    float phase = 0, ratio = 1, targetRatio = 1;
};
SW_ENTRY(Tune)
