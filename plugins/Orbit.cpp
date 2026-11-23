// Orbit — phaser
#include "Common.h"

struct Orbit : SWEffect
{
    static Layout layout()
    {
        Layout l;
        l.add(param("rate", "Rate", 0.02f, 10, 0.4f, "Hz", 1));
        l.add(param("depth", "Depth", 0, 100, 60, "%"));
        l.add(param("centre", "Centre", 100, 8000, 1000, "Hz", 1000));
        l.add(param("fb", "Feedback", -95, 95, 40, "%"));
        l.add(param("mix", "Mix", 0, 100, 50, "%"));
        return l;
    }
    Orbit() : SWEffect(layout()) {}
    void prepareToPlay(double sr, int block) override { fx.prepare(specFor(sr, block)); fx.reset(); }
    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals nd;
        fx.setRate(get("rate")); fx.setDepth(get("depth") / 100.0f); fx.setCentreFrequency(get("centre"));
        fx.setFeedback(get("fb") / 100.0f); fx.setMix(get("mix") / 100.0f);
        juce::dsp::AudioBlock<float> b(buf);
        fx.process(juce::dsp::ProcessContextReplacing<float>(b));
    }
    juce::dsp::Phaser<float> fx;
};
SW_ENTRY(Orbit)
