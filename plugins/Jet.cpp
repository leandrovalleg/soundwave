// Jet — flanger (short modulated delay with strong +/- feedback)
#include "Common.h"

struct Jet : SWEffect
{
    static Layout layout()
    {
        Layout l;
        l.add(param("rate", "Rate", 0.02f, 5, 0.25f, "Hz", 0.5f));
        l.add(param("depth", "Depth", 0, 100, 70, "%"));
        l.add(param("delay", "Delay", 0.5f, 10, 2, "ms", 2));
        l.add(param("fb", "Feedback", -95, 95, 60, "%"));
        l.add(param("width", "Stereo Phase", 0, 180, 90, "deg"));
        l.add(param("mix", "Mix", 0, 100, 50, "%"));
        return l;
    }
    Jet() : SWEffect(layout()) {}

    void prepareToPlay(double sr, int block) override
    {
        line.prepare(specFor(sr, block)); line.setMaximumDelayInSamples((int) (0.03 * sr)); line.reset();
        last[0] = last[1] = 0;
    }
    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals nd;
        const float sr = (float) getSampleRate(), inc = get("rate") / sr;
        const float depth = get("depth") / 100.0f, centre = get("delay") * 0.001f * sr;
        const float fb = get("fb") / 100.0f, mix = get("mix") / 100.0f, off = get("width") / 360.0f;
        const int chs = juce::jmin(2, buf.getNumChannels());
        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            for (int c = 0; c < chs; ++c)
            {
                float ph = lfo + (c ? off : 0.0f); ph -= std::floor(ph);
                float tri = 1.0f - std::abs(2.0f * ph - 1.0f);                    // 0..1
                float d = juce::jmax(1.0f, centre * (1.0f + depth * (tri * 2.0f - 1.0f) * 0.95f));
                float x = buf.getSample(c, i);
                line.pushSample(c, x + last[c] * fb);
                float y = line.popSample(c, d);
                last[c] = y;
                buf.setSample(c, i, x * (1 - mix) + y * mix);
            }
            lfo += inc; if (lfo >= 1) lfo -= 1;
        }
    }
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> line { 4096 };
    float lfo = 0, last[2] {};
};
SW_ENTRY(Jet)
