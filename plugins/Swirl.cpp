// Swirl — chorus
#include "Common.h"

struct Swirl : SWEffect
{
    static Layout layout()
    {
        Layout l;
        l.add(param("rate", "Rate", 0.05f, 5, 0.6f, "Hz", 1));
        l.add(param("depth", "Depth", 0, 100, 35, "%"));
        l.add(param("delay", "Delay", 5, 30, 12, "ms"));
        l.add(param("fb", "Feedback", 0, 50, 0, "%"));
        l.add(param("mix", "Mix", 0, 100, 50, "%"));
        return l;
    }
    Swirl() : SWEffect(layout()) {}
    void prepareToPlay(double sr, int block) override { fx.prepare(specFor(sr, block)); fx.reset(); }
    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals nd;
        fx.setRate(get("rate")); fx.setDepth(get("depth") / 100.0f); fx.setCentreDelay(get("delay"));
        fx.setFeedback(get("fb") / 100.0f); fx.setMix(get("mix") / 100.0f);
        juce::dsp::AudioBlock<float> b(buf);
        fx.process(juce::dsp::ProcessContextReplacing<float>(b));
    }
    juce::dsp::Chorus<float> fx;
};
SW_ENTRY(Swirl)
