// Space — algorithmic reverb with pre-delay and low cut on the tail
#include "Common.h"

struct Space : SWEffect
{
    static Layout layout()
    {
        Layout l;
        l.add(param("size", "Size", 0, 100, 60, "%"));
        l.add(param("damp", "Damping", 0, 100, 40, "%"));
        l.add(param("width", "Width", 0, 100, 100, "%"));
        l.add(param("pre", "Pre-Delay", 0, 200, 20, "ms"));
        l.add(param("lowcut", "Low Cut", 20, 1000, 150, "Hz", 150));
        l.add(param("mix", "Mix", 0, 100, 30, "%"));
        return l;
    }
    Space() : SWEffect(layout()) {}

    double getTailLengthSeconds() const override { return 8.0; }

    void prepareToPlay(double sr, int block) override
    {
        auto spec = specFor(sr, block);
        verb.prepare(spec); verb.reset();
        pre.prepare(spec); pre.setMaximumDelayInSamples((int) (0.25 * sr));
        lowcut.prepare(spec);
        dryWet.prepare(spec);
        wet.setSize((int) spec.numChannels, block);
    }

    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals nd;
        juce::dsp::Reverb::Parameters p;
        p.roomSize = get("size") / 100.0f; p.damping = get("damp") / 100.0f;
        p.width = get("width") / 100.0f; p.wetLevel = 1.0f; p.dryLevel = 0.0f;
        verb.setParameters(p);
        pre.setDelay(get("pre") * 0.001f * (float) getSampleRate());
        *lowcut.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), get("lowcut"));
        dryWet.setWetMixProportion(get("mix") / 100.0f);

        juce::dsp::AudioBlock<float> block(buf);
        dryWet.pushDrySamples(block);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        pre.process(ctx); lowcut.process(ctx); verb.process(ctx);
        dryWet.mixWetSamples(block);
    }

    juce::dsp::Reverb verb;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> pre { 48000 };
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowcut;
    juce::dsp::DryWetMixer<float> dryWet;
    juce::AudioBuffer<float> wet;
};
SW_ENTRY(Space)
