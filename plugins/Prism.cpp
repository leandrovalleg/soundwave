// Prism — 6-band parametric EQ
#include "Common.h"

struct Prism : SWEffect, HasResponse
{
    using Filter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    static constexpr int N = 6;

    static Layout layout()
    {
        Layout l;
        l.add(choice("lc_on", "Low Cut", {"Off", "On"}, 0));
        l.add(param("lc_f", "Low Cut Freq", 20, 1000, 30, "Hz", 100));
        l.add(param("ls_f", "Low Shelf Freq", 20, 1000, 100, "Hz", 150));
        l.add(param("ls_g", "Low Shelf Gain", -18, 18, 0, "dB"));
        l.add(param("b1_f", "Bell 1 Freq", 40, 8000, 400, "Hz", 600));
        l.add(param("b1_g", "Bell 1 Gain", -18, 18, 0, "dB"));
        l.add(param("b1_q", "Bell 1 Q", 0.2f, 10, 1, "", 1.5f));
        l.add(param("b2_f", "Bell 2 Freq", 200, 16000, 2500, "Hz", 2500));
        l.add(param("b2_g", "Bell 2 Gain", -18, 18, 0, "dB"));
        l.add(param("b2_q", "Bell 2 Q", 0.2f, 10, 1, "", 1.5f));
        l.add(param("hs_f", "High Shelf Freq", 1000, 20000, 8000, "Hz", 6000));
        l.add(param("hs_g", "High Shelf Gain", -18, 18, 0, "dB"));
        l.add(choice("hc_on", "High Cut", {"Off", "On"}, 0));
        l.add(param("hc_f", "High Cut Freq", 1000, 20000, 18000, "Hz", 8000));
        l.add(param("out", "Output", -18, 18, 0, "dB"));
        return l;
    }

    Prism() : SWEffect(layout()) {}

    void prepareToPlay(double sr, int block) override
    {
        auto spec = specFor(sr, block);
        for (auto& f : bands) { f.prepare(spec); f.reset(); }
        out.prepare(spec); out.setRampDurationSeconds(0.02);
    }

    void update()
    {
        using C = juce::dsp::IIR::Coefficients<float>;
        auto sr = getSampleRate();
        auto db = [](float g) { return juce::Decibels::decibelsToGain(g); };
        *bands[0].state = *C::makeHighPass(sr, get("lc_f"));
        *bands[1].state = *C::makeLowShelf(sr, get("ls_f"), 0.707f, db(get("ls_g")));
        *bands[2].state = *C::makePeakFilter(sr, get("b1_f"), get("b1_q"), db(get("b1_g")));
        *bands[3].state = *C::makePeakFilter(sr, get("b2_f"), get("b2_q"), db(get("b2_g")));
        *bands[4].state = *C::makeHighShelf(sr, juce::jmin(get("hs_f"), (float) sr * 0.45f), 0.707f, db(get("hs_g")));
        *bands[5].state = *C::makeLowPass(sr, juce::jmin(get("hc_f"), (float) sr * 0.45f));
        out.setGainDecibels(get("out"));
    }

    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals nd;
        update();
        juce::dsp::AudioBlock<float> block(buf);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        const bool on[N] = { geti("lc_on") == 1, get("ls_g") != 0, get("b1_g") != 0,
                             get("b2_g") != 0, get("hs_g") != 0, geti("hc_on") == 1 };
        for (int i = 0; i < N; ++i)
            if (on[i]) bands[i].process(ctx);
        out.process(ctx);
    }

    // Editor curve: same coefficients as update(), built from current params (message thread).
    void responseDb(const double* hz, float* db, int n) const override
    {
        using C = juce::dsp::IIR::Coefficients<float>;
        auto sr = getSampleRate() > 0 ? getSampleRate() : 48000.0;
        auto g = [](float d) { return juce::Decibels::decibelsToGain(d); };
        juce::ReferenceCountedArray<C> cs;
        if (geti("lc_on") == 1) cs.add(C::makeHighPass(sr, get("lc_f")));
        cs.add(C::makeLowShelf(sr, get("ls_f"), 0.707f, g(get("ls_g"))));
        cs.add(C::makePeakFilter(sr, get("b1_f"), get("b1_q"), g(get("b1_g"))));
        cs.add(C::makePeakFilter(sr, get("b2_f"), get("b2_q"), g(get("b2_g"))));
        cs.add(C::makeHighShelf(sr, juce::jmin(get("hs_f"), (float) sr * 0.45f), 0.707f, g(get("hs_g"))));
        if (geti("hc_on") == 1) cs.add(C::makeLowPass(sr, juce::jmin(get("hc_f"), (float) sr * 0.45f)));
        for (int i = 0; i < n; ++i)
        {
            double m = juce::Decibels::decibelsToGain((double) get("out"));
            for (auto* c : cs) m *= c->getMagnitudeForFrequency(juce::jmin(hz[i], sr * 0.49), sr);
            db[i] = (float) juce::Decibels::gainToDecibels(m, -100.0);
        }
    }

    Filter bands[N];
    juce::dsp::Gain<float> out;
};
SW_ENTRY(Prism)
