// Echo — stereo / ping-pong delay with tempo sync and filtered feedback
#include "Common.h"

struct Echo : SWEffect
{
    static Layout layout()
    {
        Layout l;
        l.add(choice("sync", "Sync", {"Free", "1/4", "1/8", "1/8 dotted", "1/8 triplet", "1/16", "1/2"}, 2));
        l.add(param("time", "Time (Free)", 1, 2000, 350, "ms", 300));
        l.add(param("fb", "Feedback", 0, 95, 40, "%"));
        l.add(choice("pp", "Ping-Pong", {"Off", "On"}, 1));
        l.add(param("hc", "Feedback High Cut", 1000, 20000, 6000, "Hz", 5000));
        l.add(param("lc", "Feedback Low Cut", 20, 2000, 120, "Hz", 200));
        l.add(param("mix", "Mix", 0, 100, 25, "%"));
        return l;
    }
    Echo() : SWEffect(layout()) {}

    double getTailLengthSeconds() const override { return 10.0; }

    void prepareToPlay(double sr, int block) override
    {
        line.prepare(specFor(sr, block));
        line.setMaximumDelayInSamples((int) (2.5 * sr));
        line.reset();
        for (auto& f : hc) f.reset();
        for (auto& f : lc) f.reset();
        delaySmooth.reset(sr, 0.05);
    }

    float delayMs()
    {
        static const float beats[] = { 0, 1.0f, 0.5f, 0.75f, 1.0f / 3.0f, 0.25f, 2.0f };
        int s = geti("sync");
        if (s == 0) return get("time");
        double bpm = 120;
        if (auto* ph = getPlayHead())
            if (auto pos = ph->getPosition())
                if (auto b = pos->getBpm()) bpm = *b;
        return (float) (60000.0 / bpm * beats[s]);
    }

    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals nd;
        using C = juce::dsp::IIR::Coefficients<float>;
        const double sr = getSampleRate();
        auto hcC = C::makeFirstOrderLowPass(sr, juce::jmin(get("hc"), (float) sr * 0.45f));
        auto lcC = C::makeFirstOrderHighPass(sr, get("lc"));
        for (int c = 0; c < 2; ++c) { hc[c].coefficients = hcC; lc[c].coefficients = lcC; }
        delaySmooth.setTargetValue(juce::jmin(delayMs() * 0.001f * (float) sr, (float) (2.4 * sr)));
        const float fb = get("fb") / 100.0f, mix = get("mix") / 100.0f;
        const bool pp = geti("pp") == 1 && buf.getNumChannels() > 1;
        const int chs = juce::jmin(2, buf.getNumChannels());

        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            line.setDelay(delaySmooth.getNextValue());
            float in[2] = { buf.getSample(0, i), buf.getSample(chs - 1, i) };
            float d[2];
            for (int c = 0; c < chs; ++c) d[c] = line.popSample(c);
            if (chs == 1) d[1] = d[0];
            for (int c = 0; c < chs; ++c)
            {
                float fbSig = pp ? d[1 - c] : d[c];
                float src = pp ? (c == 0 ? 0.5f * (in[0] + in[1]) : 0.0f) : in[c];
                float w = lc[c].processSample(hc[c].processSample(src + fbSig * fb));
                line.pushSample(c, w);
                buf.setSample(c, i, in[c] * (1.0f - mix) + d[c] * mix);
            }
        }
    }

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> line { 192000 };
    juce::dsp::IIR::Filter<float> hc[2], lc[2];
    juce::SmoothedValue<float> delaySmooth;
};
SW_ENTRY(Echo)
