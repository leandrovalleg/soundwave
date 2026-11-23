// Gate — noise gate
#include "Common.h"

struct Gate : SWEffect
{
    static Layout layout()
    {
        Layout l;
        l.add(param("thr", "Threshold", -80, 0, -40, "dB"));
        l.add(param("ratio", "Ratio", 1, 50, 10, ":1", 8));
        l.add(param("att", "Attack", 0.1f, 50, 1, "ms", 5));
        l.add(param("rel", "Release", 5, 1000, 100, "ms", 150));
        return l;
    }
    Gate() : SWEffect(layout()) {}

    void prepareToPlay(double sr, int block) override { gate.prepare(specFor(sr, block)); }
    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals nd;
        gate.setThreshold(get("thr")); gate.setRatio(get("ratio"));
        gate.setAttack(get("att")); gate.setRelease(get("rel"));
        juce::dsp::AudioBlock<float> block(buf);
        gate.process(juce::dsp::ProcessContextReplacing<float>(block));
    }
    juce::dsp::NoiseGate<float> gate;
};
SW_ENTRY(Gate)
