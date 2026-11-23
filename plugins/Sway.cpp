// Sway — auto-pan / tremolo, free or tempo-synced
#include "Common.h"

struct Sway : SWEffect
{
    static Layout layout()
    {
        Layout l;
        l.add(choice("sync", "Sync", { "Free", "1/1", "1/2", "1/4", "1/8", "1/16" }, 3));
        l.add(param("rate", "Rate (Free)", 0.05f, 20, 2, "Hz", 2));
        l.add(param("depth", "Depth", 0, 100, 80, "%"));
        l.add(choice("shape", "Shape", { "Sine", "Triangle", "Square" }, 0));
        l.add(choice("mode", "Mode", { "Pan", "Tremolo" }, 0));
        return l;
    }
    Sway() : SWEffect(layout()) {}
    void prepareToPlay(double, int) override { phase = 0; }

    float rateHz()
    {
        static const float beats[] = { 0, 4, 2, 1, 0.5f, 0.25f };
        int s = geti("sync"); if (s == 0) return get("rate");
        double bpm = 120;
        if (auto* ph = getPlayHead()) if (auto pos = ph->getPosition()) if (auto b = pos->getBpm()) bpm = *b;
        return (float) (bpm / 60.0 / beats[s]);
    }

    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        const float inc = rateHz() / (float) getSampleRate(), depth = get("depth") / 100.0f;
        const int shape = geti("shape"); const bool pan = geti("mode") == 0 && buf.getNumChannels() > 1;
        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            float m = shape == 0 ? std::sin(phase * juce::MathConstants<float>::twoPi)
                    : shape == 1 ? 1.0f - 4.0f * std::abs(phase - 0.5f)
                    : (phase < 0.5f ? 1.0f : -1.0f);                                // -1..1
            if (pan)
            {
                float p = 0.5f + 0.5f * m * depth;                                  // equal-power
                buf.setSample(0, i, buf.getSample(0, i) * std::cos(p * juce::MathConstants<float>::halfPi) * 1.414f);
                buf.setSample(1, i, buf.getSample(1, i) * std::sin(p * juce::MathConstants<float>::halfPi) * 1.414f);
            }
            else
            {
                float g = 1.0f - depth * (0.5f - 0.5f * m);
                for (int c = 0; c < buf.getNumChannels(); ++c) buf.setSample(c, i, buf.getSample(c, i) * g);
            }
            phase += inc; if (phase >= 1) phase -= 1;
        }
    }
    float phase = 0;
};
SW_ENTRY(Sway)
