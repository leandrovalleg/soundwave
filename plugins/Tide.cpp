// Tide — wavetable synth: 2 morphing wavetable oscillators, unison, SVF filter, 2 envelopes, LFO.
#include "Common.h"

namespace tide {

constexpr int TableSize = 2048, Frames = 64;

// Built-in tables, generated additively (band-limited per frame at build time).
// Each "bank" morphs across Frames frames as Position goes 0..1.
struct Bank { std::vector<float> data; float at(int f, int i) const { return data[(size_t) f * TableSize + (size_t) i]; } };

inline Bank makeBank(int kind)
{
    Bank b; b.data.resize((size_t) Frames * TableSize);
    const int maxH = 512;
    for (int f = 0; f < Frames; ++f)
    {
        const float m = f / (float) (Frames - 1);
        std::vector<float> amp(maxH + 1, 0.0f), ph(maxH + 1, 0.0f);
        for (int h = 1; h <= maxH; ++h)
        {
            float saw = 1.0f / h, sq = (h % 2) ? 1.0f / h : 0.0f;
            switch (kind)
            {
                case 0: amp[h] = (h == 1 ? 1.0f - m : 0.0f) + m * saw; break;                      // Sine -> Saw
                case 1: amp[h] = (1 - m) * sq + m * (h % 2 ? 0.0f : 1.0f / h); break;               // Square -> even-saw (hollow->nasal)
                case 2: { float c = 1 + m * 40, w = 3 + m * 6;                                      // Formant sweep (vowel-ish)
                          amp[h] = std::exp(-0.5f * std::pow((h - c) / w, 2.0f)) / std::sqrt((float) h); } break;
                case 3: amp[h] = saw * (0.5f + 0.5f * std::cos(h * m * 0.7f));                       // Comb sweep (phasing saw)
                        break;
                default: { juce::Random r(h * 7919 + 13); float v = r.nextFloat();                  // Digital: random spectra
                           amp[h] = (v < 0.15f + 0.6f * m ? v : 0.0f) / std::sqrt((float) h); ph[h] = r.nextFloat() * 6.2832f; }
            }
        }
        float peak = 0;
        auto* out = &b.data[(size_t) f * TableSize];
        for (int i = 0; i < TableSize; ++i)
        {
            float x = 2.0f * juce::MathConstants<float>::pi * i / TableSize, s = 0;
            for (int h = 1; h <= maxH; ++h) if (amp[h] != 0.0f) s += amp[h] * std::sin(h * x + ph[h]);
            out[i] = s; peak = juce::jmax(peak, std::abs(s));
        }
        if (peak > 0) for (int i = 0; i < TableSize; ++i) out[i] /= peak;
    }
    return b;
}

inline const juce::StringArray bankNames { "Basic", "Hollow", "Vowel", "Comb", "Digital" };

struct Banks { std::vector<Bank> b; Banks() { for (int i = 0; i < bankNames.size(); ++i) b.push_back(makeBank(i)); } };
inline const Banks& banks() { static Banks bk; return bk; }

// ponytail: tables are built with 512 harmonics, so very high notes alias a little; add per-octave mipmaps if it bothers
inline float read(const Bank& bank, float pos, double phase)
{
    float fp = pos * (Frames - 1); int f0 = (int) fp, f1 = juce::jmin(f0 + 1, Frames - 1); float ff = fp - f0;
    double idx = phase * TableSize; int i0 = (int) idx; float fi = (float) (idx - i0); int i1 = (i0 + 1) & (TableSize - 1);
    float a = bank.at(f0, i0) + fi * (bank.at(f0, i1) - bank.at(f0, i0));
    float b = bank.at(f1, i0) + fi * (bank.at(f1, i1) - bank.at(f1, i0));
    return a + ff * (b - a);
}

struct Params
{
    int bank[2]; float pos[2], level[2], semi[2], detune, voices, spread;
    int ftype; float cutoff, res, fEnvAmt, keytrack;
    juce::ADSR::Parameters amp, fenv;
    float lfoRate, lfoPos, lfoCut, glide, gain;
};

struct Voice : juce::SynthesiserVoice
{
    static constexpr int MaxUni = 7;
    Params* p = nullptr;
    juce::ADSR ampEnv, fEnv;
    juce::dsp::StateVariableTPTFilter<float> filter;
    double phase[2][MaxUni] {};
    float note = 60, vel = 0, lfoPhase = 0;

    bool canPlaySound(juce::SynthesiserSound*) override { return true; }
    void prepare(double sr, int block)
    {
        ampEnv.setSampleRate(sr); fEnv.setSampleRate(sr);
        filter.prepare({ sr, (juce::uint32) block, 2 });
    }
    void startNote(int n, float v, juce::SynthesiserSound*, int) override
    {
        note = (float) n; vel = v;
        juce::Random r;
        for (auto& o : phase) for (auto& ph : o) ph = r.nextDouble();
        filter.reset();
        ampEnv.setParameters(p->amp); fEnv.setParameters(p->fenv);
        ampEnv.noteOn(); fEnv.noteOn();
    }
    void stopNote(float, bool tail) override
    {
        if (tail) { ampEnv.noteOff(); fEnv.noteOff(); }
        else { clearCurrentNote(); ampEnv.reset(); }
    }
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>& out, int start, int num) override
    {
        if (!isVoiceActive()) return;
        const double sr = getSampleRate();
        const int uni = juce::jlimit(1, MaxUni, (int) p->voices);
        const auto& bk = banks().b;
        filter.setType((juce::dsp::StateVariableTPTFilterType) p->ftype);
        filter.setResonance(0.5f + p->res * 9.5f);
        const float lfoInc = p->lfoRate / (float) sr;

        for (int i = 0; i < num; ++i)
        {
            const float lfo = std::sin(lfoPhase * juce::MathConstants<float>::twoPi);
            lfoPhase += lfoInc; if (lfoPhase >= 1) lfoPhase -= 1;
            float L = 0, R = 0;
            for (int o = 0; o < 2; ++o)
            {
                if (p->level[o] <= 0) continue;
                const float pos = juce::jlimit(0.0f, 1.0f, p->pos[o] + lfo * p->lfoPos);
                const auto& bank = bk[(size_t) p->bank[o]];
                for (int u = 0; u < uni; ++u)
                {
                    const float spreadPos = uni == 1 ? 0.0f : (u / (float) (uni - 1)) * 2.0f - 1.0f; // -1..1
                    const float cents = spreadPos * p->detune * 50.0f;
                    const double hz = 440.0 * std::pow(2.0, (note + p->semi[o] - 69.0 + cents / 100.0) / 12.0);
                    const float s = read(bank, pos, phase[o][u]) * p->level[o];
                    phase[o][u] += hz / sr; if (phase[o][u] >= 1) phase[o][u] -= 1;
                    const float pan = 0.5f + 0.5f * spreadPos * p->spread;
                    L += s * (1 - pan); R += s * pan;
                }
            }
            const float norm = 1.4f / std::sqrt((float) uni);
            const float fe = fEnv.getNextSample();
            const float cutSemis = p->fEnvAmt * 96.0f * fe + p->lfoCut * 48.0f * lfo + p->keytrack * (note - 60.0f);
            filter.setCutoffFrequency(juce::jlimit(20.0f, (float) sr * 0.45f, p->cutoff * std::pow(2.0f, cutSemis / 12.0f)));
            const float a = ampEnv.getNextSample() * vel * norm;
            out.addSample(0, start + i, filter.processSample(0, L) * a);
            if (out.getNumChannels() > 1) out.addSample(1, start + i, filter.processSample(1, R) * a);
            if (!ampEnv.isActive()) { clearCurrentNote(); break; }
        }
    }
};

struct Sound : juce::SynthesiserSound
{
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

} // namespace tide

struct Tide : SWEffect
{
    static Layout layout()
    {
        Layout l;
        for (int o = 1; o <= 2; ++o)
        {
            auto n = juce::String(o);
            l.add(choice("o" + n + "_bank", "Osc " + n + " Table", tide::bankNames, o == 1 ? 0 : 2));
            l.add(param("o" + n + "_pos", "Osc " + n + " Position", 0, 1, o == 1 ? 0.6f : 0.3f));
            l.add(param("o" + n + "_lvl", "Osc " + n + " Level", 0, 1, o == 1 ? 0.8f : 0.0f));
            l.add(param("o" + n + "_semi", "Osc " + n + " Semi", -24, 24, 0, "st"));
        }
        l.add(param("uni", "Unison Voices", 1, 7, 3));
        l.add(param("detune", "Detune", 0, 1, 0.25f));
        l.add(param("spread", "Stereo Spread", 0, 1, 0.8f));
        l.add(choice("ftype", "Filter Type", { "Low Pass", "Band Pass", "High Pass" }, 0));
        l.add(param("cut", "Cutoff", 20, 20000, 2000, "Hz", 1000));
        l.add(param("res", "Resonance", 0, 1, 0.2f));
        l.add(param("fenv", "Filter Env Amount", -1, 1, 0.3f));
        l.add(param("ktrk", "Key Track", 0, 1, 0.5f));
        l.add(param("a", "Amp Attack", 0.001f, 5, 0.005f, "s", 0.3f));
        l.add(param("d", "Amp Decay", 0.001f, 5, 0.3f, "s", 0.5f));
        l.add(param("s", "Amp Sustain", 0, 1, 0.8f));
        l.add(param("r", "Amp Release", 0.001f, 8, 0.4f, "s", 0.6f));
        l.add(param("fa", "Filter Attack", 0.001f, 5, 0.01f, "s", 0.3f));
        l.add(param("fd", "Filter Decay", 0.001f, 5, 0.5f, "s", 0.5f));
        l.add(param("fs", "Filter Sustain", 0, 1, 0.2f));
        l.add(param("fr", "Filter Release", 0.001f, 8, 0.5f, "s", 0.6f));
        l.add(param("lrate", "LFO Rate", 0.01f, 20, 0.5f, "Hz", 2));
        l.add(param("lpos", "LFO > Position", 0, 0.5f, 0.1f));
        l.add(param("lcut", "LFO > Cutoff", 0, 1, 0));
        l.add(param("gain", "Volume", -36, 6, -6, "dB"));
        return l;
    }

    Tide() : SWEffect(layout(), true)
    {
        for (int i = 0; i < 16; ++i) { auto* v = new tide::Voice(); v->p = &p; synth.addVoice(v); }
        synth.addSound(new tide::Sound());
        tide::banks(); // build tables up front, not on the audio thread
    }

    bool acceptsMidi() const override { return true; }
    double getTailLengthSeconds() const override { return 8.0; }

    void prepareToPlay(double sr, int block) override
    {
        synth.setCurrentPlaybackSampleRate(sr);
        for (int i = 0; i < synth.getNumVoices(); ++i)
            static_cast<tide::Voice*>(synth.getVoice(i))->prepare(sr, block);
    }

    void readParams()
    {
        for (int o = 0; o < 2; ++o)
        {
            auto n = "o" + juce::String(o + 1);
            p.bank[o]  = (int) state.getRawParameterValue(n + "_bank")->load();
            p.pos[o]   = state.getRawParameterValue(n + "_pos")->load();
            p.level[o] = state.getRawParameterValue(n + "_lvl")->load();
            p.semi[o]  = std::round(state.getRawParameterValue(n + "_semi")->load());
        }
        p.voices = std::round(get("uni")); p.detune = get("detune"); p.spread = get("spread");
        p.ftype = geti("ftype"); p.cutoff = get("cut"); p.res = get("res"); p.fEnvAmt = get("fenv"); p.keytrack = get("ktrk");
        p.amp  = { get("a"), get("d"), get("s"), get("r") };
        p.fenv = { get("fa"), get("fd"), get("fs"), get("fr") };
        p.lfoRate = get("lrate"); p.lfoPos = get("lpos"); p.lfoCut = get("lcut");
    }

    void processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer& midi) override
    {
        juce::ScopedNoDenormals nd;
        readParams();
        buf.clear();
        synth.renderNextBlock(buf, midi, 0, buf.getNumSamples());
        buf.applyGain(juce::Decibels::decibelsToGain(get("gain")));
    }

    tide::Params p {};
    juce::Synthesiser synth;
};
SW_ENTRY(Tide)
