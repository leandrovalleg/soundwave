// Loads every SW VST3, feeds 2s of noisy sine, checks output is finite, non-silent and bounded.
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_events/juce_events.h>

int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    juce::VST3PluginFormat fmt;
    int failed = 0;
    for (int a = 1; a < argc; ++a)
    {
        juce::OwnedArray<juce::PluginDescription> found;
        fmt.findAllTypesForFile(found, argv[a]);
        if (found.isEmpty()) { printf("FAIL load %s\n", argv[a]); ++failed; continue; }
        juce::String err;
        auto p = fmt.createInstanceFromDescription(*found[0], 48000, 512, err);
        if (!p) { printf("FAIL create %s: %s\n", argv[a], err.toRawUTF8()); ++failed; continue; }
        p->setPlayConfigDetails(2, 2, 48000, 512);
        p->prepareToPlay(48000, 512);
        juce::AudioBuffer<float> buf(2, 512); juce::MidiBuffer midi;
        juce::Random rnd(1);
        double phase = 0, energy = 0; float peak = 0; bool finite = true;
        for (int b = 0; b < 190; ++b)
        {
            for (int i = 0; i < 512; ++i, phase += 2 * juce::MathConstants<double>::pi * 220 / 48000)
                for (int c = 0; c < 2; ++c)
                    buf.setSample(c, i, (b < 95 ? 0.5f : 0.0f) * (float) std::sin(phase) + 0.05f * (rnd.nextFloat() - 0.5f));
            p->processBlock(buf, midi);
            for (int c = 0; c < 2; ++c)
                for (int i = 0; i < 512; ++i)
                {
                    float v = buf.getSample(c, i);
                    finite &= std::isfinite(v); peak = std::max(peak, std::abs(v)); energy += v * v;
                }
        }
        // state round-trip
        juce::MemoryBlock st; p->getStateInformation(st); p->setStateInformation(st.getData(), (int) st.getSize());
        bool ok = finite && energy > 1.0 && peak < 4.0f;
        printf("%s %-10s params=%2d peak=%.2f\n", ok ? "OK  " : "FAIL", p->getName().toRawUTF8(),
               p->getParameters().size(), peak);
        failed += !ok;
        p->releaseResources();
    }
    return failed;
}
