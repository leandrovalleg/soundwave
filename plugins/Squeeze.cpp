// Squeeze — compressor with parallel mix
#include "Common.h"

struct Squeeze : SWEffect
{
    static Layout layout()
    {
        Layout l;
        l.add(param("thr", "Threshold", -60, 0, -18, "dB"));
        l.add(param("ratio", "Ratio", 1, 20, 4, ":1", 4));
        l.add(param("att", "Attack", 0.1f, 100, 10, "ms", 10));
        l.add(param("rel", "Release", 10, 1000, 120, "ms", 150));
        l.add(param("makeup", "Makeup", 0, 24, 0, "dB"));
        l.add(param("mix", "Mix", 0, 100, 100, "%"));
        return l;
    }
    Squeeze() : SWEffect(layout()) {}

    void prepareToPlay(double sr, int block) override
    {
        auto spec = specFor(sr, block);
        comp.prepare(spec); makeup.prepare(spec); dryWet.prepare(spec);
        makeup.setRampDurationSeconds(0.02);
    }

    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals nd;
        comp.setThreshold(get("thr")); comp.setRatio(get("ratio"));
        comp.setAttack(get("att")); comp.setRelease(get("rel"));
        makeup.setGainDecibels(get("makeup"));
        dryWet.setWetMixProportion(get("mix") / 100.0f);

        juce::dsp::AudioBlock<float> block(buf);
        dryWet.pushDrySamples(block);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        comp.process(ctx); makeup.process(ctx);
        dryWet.mixWetSamples(block);
    }

    juce::dsp::Compressor<float> comp;
    juce::dsp::Gain<float> makeup;
    juce::dsp::DryWetMixer<float> dryWet;
};
SW_ENTRY(Squeeze)
