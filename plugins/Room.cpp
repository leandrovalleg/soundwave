// Room — convolution reverb. Loads any WAV/AIFF impulse response; ships with 3 generated spaces.
#include "Common.h"

struct Room : SWEffect, LoadsFile
{
    static Layout layout()
    {
        Layout l;
        l.add(choice("space", "Space", { "Small Room", "Hall", "Plate", "Custom File" }, 1));
        l.add(param("pre", "Pre-Delay", 0, 200, 10, "ms"));
        l.add(param("lowcut", "Low Cut", 20, 1000, 120, "Hz", 150));
        l.add(param("highcut", "High Cut", 1000, 20000, 12000, "Hz", 6000));
        l.add(param("mix", "Mix", 0, 100, 30, "%"));
        return l;
    }
    Room() : SWEffect(layout()) {}
    double getTailLengthSeconds() const override { return 6.0; }

    // Custom IR path is kept in the saved state so projects reopen with it.
    void loadImpulseFile(const juce::File& f) override
    {
        state.state.setProperty("irPath", f.getFullPathName(), nullptr);
        loadedSpace = -1;
    }

    static juce::AudioBuffer<float> makeIR(int kind, double sr)
    {
        const float secs[] = { 0.6f, 2.8f, 1.8f }, damp[] = { 6000, 3500, 9000 };
        const int n = (int) (secs[kind] * sr);
        juce::AudioBuffer<float> ir(2, n);
        juce::Random r(kind + 1);
        for (int c = 0; c < 2; ++c)
        {
            float lp = 0; const float a = std::exp(-2.0f * juce::MathConstants<float>::pi * damp[kind] / (float) sr);
            for (int i = 0; i < n; ++i)
            {
                float t = i / (float) n, env = std::exp(-6.9f * t);                 // -60 dB at the end
                float noise = r.nextFloat() * 2 - 1;
                lp = (1 - a) * noise + a * lp;
                float s = (kind == 2 ? noise : lp) * env;
                if (kind == 0 && i < sr * 0.03 && r.nextFloat() < 0.01f) s += 0.8f * env; // early reflections
                ir.setSample(c, i, s);
            }
        }
        return ir;
    }

    void updateIR()
    {
        int s = geti("space");
        auto path = state.state.getProperty("irPath").toString();
        if (s == loadedSpace && (s != 3 || path == loadedPath)) return;
        loadedSpace = s; loadedPath = path;
        if (s == 3)
        {
            juce::File f(path);
            if (f.existsAsFile())
                conv.loadImpulseResponse(f, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::yes, 0);
            return;
        }
        conv.loadImpulseResponse(makeIR(s, getSampleRate()), getSampleRate(),
                                 juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no,
                                 juce::dsp::Convolution::Normalise::yes);
    }

    void prepareToPlay(double sr, int block) override
    {
        auto spec = specFor(sr, block);
        conv.prepare(spec); pre.prepare(spec); pre.setMaximumDelayInSamples((int) (0.25 * sr));
        lc.prepare(spec); hc.prepare(spec); dryWet.prepare(spec);
        loadedSpace = -1; updateIR();
    }

    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals nd;
        using C = juce::dsp::IIR::Coefficients<float>;
        const double sr = getSampleRate();
        updateIR(); // ponytail: IR swap is async inside juce::dsp::Convolution, so this is RT-safe
        pre.setDelay(get("pre") * 0.001f * (float) sr);
        *lc.state = *C::makeHighPass(sr, get("lowcut"));
        *hc.state = *C::makeLowPass(sr, juce::jmin(get("highcut"), (float) sr * 0.45f));
        dryWet.setWetMixProportion(get("mix") / 100.0f);
        juce::dsp::AudioBlock<float> b(buf);
        dryWet.pushDrySamples(b);
        juce::dsp::ProcessContextReplacing<float> ctx(b);
        pre.process(ctx); conv.process(ctx); lc.process(ctx); hc.process(ctx);
        dryWet.mixWetSamples(b);
    }

    juce::dsp::Convolution conv;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> pre { 48000 };
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lc, hc;
    juce::dsp::DryWetMixer<float> dryWet;
    int loadedSpace = -1; juce::String loadedPath;
};
SW_ENTRY(Room)
