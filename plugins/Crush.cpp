// Crush — bitcrusher / sample-rate reducer
#include "Common.h"

struct Crush : SWEffect
{
    static Layout layout()
    {
        Layout l;
        l.add(param("bits", "Bit Depth", 1, 16, 8, "bit"));
        l.add(param("rate", "Sample Rate", 200, 48000, 11025, "Hz", 6000));
        l.add(param("jitter", "Jitter", 0, 100, 0, "%"));
        l.add(choice("filt", "Post Filter", { "Off", "On" }, 0));
        l.add(param("mix", "Mix", 0, 100, 100, "%"));
        l.add(param("out", "Output", -24, 12, 0, "dB"));
        return l;
    }
    Crush() : SWEffect(layout()) {}

    void prepareToPlay(double sr, int block) override
    {
        post.prepare(specFor(sr, block)); post.reset();
        counter = 1; held[0] = held[1] = 0;
    }

    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals nd;
        const double sr = getSampleRate();
        const float levels = std::pow(2.0f, get("bits") - 1.0f);          // continuous bit depth
        const float step = (float) juce::jmax(1.0, sr / get("rate"));
        const float jit = get("jitter") / 100.0f, mix = get("mix") / 100.0f;
        const float out = juce::Decibels::decibelsToGain(get("out"));
        const int chs = juce::jmin(2, buf.getNumChannels());

        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            if ((counter -= 1.0f) <= 0.0f)
            {
                counter += step * (1.0f + jit * (rnd.nextFloat() - 0.5f));
                for (int c = 0; c < chs; ++c)
                    held[c] = std::round(buf.getSample(c, i) * levels) / levels;
            }
            for (int c = 0; c < chs; ++c)
                buf.setSample(c, i, (buf.getSample(c, i) * (1 - mix) + held[c] * mix) * out);
        }
        if (geti("filt") == 1)
        {
            *post.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, juce::jmin(get("rate") * 0.45f, (float) sr * 0.45f));
            juce::dsp::AudioBlock<float> b(buf);
            post.process(juce::dsp::ProcessContextReplacing<float>(b));
        }
    }

    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> post;
    juce::Random rnd;
    float counter = 1, held[2] {};
};
SW_ENTRY(Crush)
