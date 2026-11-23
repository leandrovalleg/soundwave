#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "UI.h"

using Layout = juce::AudioProcessorValueTreeState::ParameterLayout;
using Range  = juce::NormalisableRange<float>;

inline std::unique_ptr<juce::AudioParameterFloat>
param(const juce::String& id, const juce::String& name, float lo, float hi, float def,
      const juce::String& unit = {}, float skewCentre = 0.0f)
{
    Range r(lo, hi);
    if (skewCentre > 0.0f) r.setSkewForCentre(skewCentre);
    return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id, 1}, name, r, def,
        juce::AudioParameterFloatAttributes().withLabel(unit));
}

inline std::unique_ptr<juce::AudioParameterChoice>
choice(const juce::String& id, const juce::String& name, juce::StringArray opts, int def = 0)
{
    return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id, 1}, name, opts, def);
}

// Base: stereo in/out effect with a parameter tree and saved state.
struct SWEffect : juce::AudioProcessor
{
    SWEffect(Layout layout, bool instrument = false)
        : juce::AudioProcessor(instrument
              ? BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)
              : BusesProperties()
                  .withInput("Input", juce::AudioChannelSet::stereo(), true)
                  .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          state(*this, nullptr, "STATE", std::move(layout)) {}

    float get(const char* id) const { return state.getRawParameterValue(id)->load(); }
    int   geti(const char* id) const { return (int) get(id); }

    bool isBusesLayoutSupported(const BusesLayout& l) const override
    {
        auto out = l.getMainOutputChannelSet();
        return (out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono())
               && (getBusCount(true) == 0 || l.getMainInputChannelSet() == out);
    }

    juce::AudioProcessorEditor* createEditor() override { return new sw::Editor(*this); }
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void releaseResources() override {}

    void getStateInformation(juce::MemoryBlock& d) override
    {
        if (auto xml = state.copyState().createXml()) copyXmlToBinary(*xml, d);
    }
    void setStateInformation(const void* data, int size) override
    {
        if (auto xml = getXmlFromBinary(data, size))
            if (xml->hasTagName(state.state.getType()))
                state.replaceState(juce::ValueTree::fromXml(*xml));
    }

    juce::dsp::ProcessSpec specFor(double sr, int block) const
    {
        return { sr, (juce::uint32) block, (juce::uint32) juce::jmax(1, getTotalNumOutputChannels()) };
    }

    juce::AudioProcessorValueTreeState state;
};

#define SW_ENTRY(Class) \
    juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Class(); }
