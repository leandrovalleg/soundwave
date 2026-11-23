// Wall — brickwall limiter for the master
#include "Common.h"

struct Wall : SWEffect
{
    static Layout layout()
    {
        Layout l;
        l.add(param("gain", "Input Gain", 0, 24, 0, "dB"));
        l.add(param("ceil", "Ceiling", -12, 0, -0.3f, "dB"));
        l.add(param("rel", "Release", 1, 500, 60, "ms", 60));
        return l;
    }
    Wall() : SWEffect(layout()) {}

    void prepareToPlay(double sr, int block) override
    {
        auto spec = specFor(sr, block);
        in.prepare(spec); lim.prepare(spec); in.setRampDurationSeconds(0.02);
    }
    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals nd;
        in.setGainDecibels(get("gain"));
        lim.setThreshold(get("ceil")); lim.setRelease(get("rel"));
        juce::dsp::AudioBlock<float> block(buf);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        in.process(ctx); lim.process(ctx);
    }
    juce::dsp::Gain<float> in;
    juce::dsp::Limiter<float> lim;
};
SW_ENTRY(Wall)
