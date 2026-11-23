// Glow — tube-style saturation (asymmetric soft clip, 4x oversampled)
#include "Common.h"

struct Glow : SWEffect
{
    static Layout layout()
    {
        Layout l;
        l.add(param("drive", "Drive", 0, 36, 6, "dB"));
        l.add(param("bias", "Warmth", 0, 100, 30, "%"));
        l.add(param("tone", "Tone", 1000, 20000, 12000, "Hz", 5000));
        l.add(param("mix", "Mix", 0, 100, 100, "%"));
        l.add(param("out", "Output", -24, 12, 0, "dB"));
        return l;
    }
    Glow() : SWEffect(layout()) {}

    void prepareToPlay(double sr, int block) override
    {
        auto spec = specFor(sr, block);
        os.initProcessing((size_t) block); os.reset();
        dc.prepare(spec); tone.prepare(spec); dryWet.prepare(spec); out.prepare(spec);
        dryWet.setMixingRule(juce::dsp::DryWetMixingRule::balanced);
        setLatencySamples((int) os.getLatencyInSamples());
        dryWet.setWetLatency(os.getLatencyInSamples());
        out.setRampDurationSeconds(0.02);
    }

    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals nd;
        using C = juce::dsp::IIR::Coefficients<float>;
        auto sr = getSampleRate();
        *dc.state   = *C::makeHighPass(sr, 15.0f);
        *tone.state = *C::makeLowPass(sr, juce::jmin(get("tone"), (float) sr * 0.45f));
        dryWet.setWetMixProportion(get("mix") / 100.0f);
        out.setGainDecibels(get("out"));

        const float g = juce::Decibels::decibelsToGain(get("drive"));
        const float b = get("bias") / 100.0f * 0.5f;        // asymmetry => even harmonics
        const float norm = 1.0f / std::tanh(g + b);          // keep level roughly constant
        const float off = std::tanh(b);

        juce::dsp::AudioBlock<float> block(buf);
        dryWet.pushDrySamples(block);
        auto up = os.processSamplesUp(block);
        for (size_t ch = 0; ch < up.getNumChannels(); ++ch)
        {
            auto* d = up.getChannelPointer(ch);
            for (size_t i = 0; i < up.getNumSamples(); ++i)
                d[i] = (std::tanh(d[i] * g + b) - off) * norm;
        }
        os.processSamplesDown(block);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        dc.process(ctx); tone.process(ctx);
        dryWet.mixWetSamples(block);
        out.process(ctx);
    }

    juce::dsp::Oversampling<float> os { 2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true };
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> dc, tone;
    juce::dsp::DryWetMixer<float> dryWet { 512 };
    juce::dsp::Gain<float> out;
};
SW_ENTRY(Glow)
