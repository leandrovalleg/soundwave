// Flux — dynamic EQ: one bell band that cuts only when that band gets loud
#include "Common.h"

struct Flux : SWEffect
{
    static Layout layout()
    {
        Layout l;
        l.add(param("freq", "Frequency", 40, 16000, 6000, "Hz", 1500));
        l.add(param("q", "Q", 0.3f, 10, 2, "", 2));
        l.add(param("thr", "Threshold", -60, 0, -24, "dB"));
        l.add(param("ratio", "Ratio", 1, 10, 4, ":1", 3));
        l.add(param("range", "Max Cut", 0, 24, 12, "dB"));
        l.add(param("att", "Attack", 0.5f, 100, 5, "ms", 10));
        l.add(param("rel", "Release", 10, 1000, 100, "ms", 150));
        l.add(choice("listen", "Listen Band", {"Off", "On"}, 0));
        return l;
    }
    Flux() : SWEffect(layout()) {}

    void prepareToPlay(double sr, int block) override
    {
        auto spec = specFor(sr, block);
        for (auto& f : det) f.reset();
        for (auto& f : eq) f.reset();
        env = 0; reduction = 0;
        juce::ignoreUnused(spec);
    }

    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals nd;
        using C = juce::dsp::IIR::Coefficients<float>;
        const double sr = getSampleRate();
        const float f = juce::jmin(get("freq"), (float) sr * 0.45f), q = get("q");
        const float thr = get("thr"), ratio = get("ratio"), range = get("range");
        const float aC = std::exp(-1.0f / (0.001f * get("att") * (float) sr));
        const float rC = std::exp(-1.0f / (0.001f * get("rel") * (float) sr));
        const bool listen = geti("listen") == 1;
        auto bp = C::makeBandPass(sr, f, q);
        const int chs = juce::jmin(buf.getNumChannels(), 2), n = buf.getNumSamples();
        for (int c = 0; c < chs; ++c) det[c].coefficients = bp;

        constexpr int sub = 32; // ponytail: EQ gain updated every 32 samples, per-sample if it ever zippers
        for (int s0 = 0; s0 < n; s0 += sub)
        {
            const int len = juce::jmin(sub, n - s0);
            for (int i = 0; i < len; ++i)
            {
                float peak = 0;
                for (int c = 0; c < chs; ++c)
                {
                    float v = det[c].processSample(buf.getSample(c, s0 + i));
                    peak = juce::jmax(peak, std::abs(v));
                    if (listen) buf.setSample(c, s0 + i, v);
                }
                env = peak > env ? aC * env + (1 - aC) * peak : rC * env + (1 - rC) * peak;
            }
            if (listen) continue;
            const float over = juce::Decibels::gainToDecibels(env, -100.0f) - thr;
            reduction = over > 0 ? juce::jmin(range, over * (1.0f - 1.0f / ratio)) : 0.0f;
            auto pk = C::makePeakFilter(sr, f, q, juce::Decibels::decibelsToGain(-reduction));
            for (int c = 0; c < chs; ++c)
            {
                eq[c].coefficients = pk;
                auto* d = buf.getWritePointer(c, s0);
                for (int i = 0; i < len; ++i) d[i] = eq[c].processSample(d[i]);
            }
        }
    }

    juce::dsp::IIR::Filter<float> det[2], eq[2];
    float env = 0, reduction = 0;
};
SW_ENTRY(Flux)
