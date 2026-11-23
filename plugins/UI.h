#pragma once
// Auto-built SoundWave plugin editor: every float param -> knob, every choice -> segmented pills.
#include <juce_audio_processors/juce_audio_processors.h>
#include "SWDesign.h"

// Optional hook: a processor that loads a file (e.g. an impulse response) gets a "LOAD" button.
// After loading, any choice param with a "Custom File" option is switched to it.
struct LoadsFile
{
    virtual void loadImpulseFile(const juce::File&) = 0;
    virtual ~LoadsFile() = default;
};

// Optional hook: a processor that can report its magnitude response gets a response-curve card.
struct HasResponse
{
    virtual void responseDb(const double* hz, float* db, int n) const = 0;  // called on the message thread
    virtual ~HasResponse() = default;
};

namespace sw
{
// "6000 Hz" -> {"6.0", "kHz"}; keeps readouts short for the dot-matrix font.
inline std::pair<juce::String, juce::String> formatValue(float v, juce::String unit)
{
    if (unit == "Hz" && std::abs(v) >= 1000.0f) { v /= 1000.0f; unit = "kHz"; }
    if (unit == "s" && std::abs(v) < 1.0f)       { v *= 1000.0f; unit = "ms"; }
    auto a = std::abs(v);
    int dec = a >= 100.0f ? 0 : a >= 10.0f ? 1 : 2;
    if (unit == "%" ) dec = 0;
    auto s = juce::String(v, dec);
    if (s == "-0" || s == "-0.0" || s == "-0.00") s = s.substring(1);
    return { s, unit };
}

struct Knob : juce::Component
{
    Knob(juce::RangedAudioParameter& p, juce::String caption) : param(p), text(caption.toUpperCase()), attach(p, slider)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setRotaryParameters(juce::degreesToRadians(225.0f), juce::degreesToRadians(495.0f), true);
        slider.setDoubleClickReturnValue(true, p.convertFrom0to1(p.getDefaultValue()));
        slider.setMouseDragSensitivity(220);
        slider.setPopupDisplayEnabled(false, false, nullptr);
        slider.onValueChange = [this] { repaint(); };
        slider.addMouseListener(this, false);
        addAndMakeVisible(slider);
    }
    void mouseEnter(const juce::MouseEvent&) override { repaint(); slider.repaint(); }
    void mouseExit(const juce::MouseEvent&) override  { repaint(); slider.repaint(); }
    void mouseUp(const juce::MouseEvent&) override    { repaint(); slider.repaint(); }

    void resized() override { slider.setBounds(getLocalBounds().removeFromTop(getWidth() * 7 / 10).reduced(4)); }

    void paint(juce::Graphics& g) override
    {
        auto r = getLocalBounds().withTrimmedTop(slider.getBottom() + 2);
        auto [num, unit] = formatValue((float) slider.getValue(), param.getLabel());
        auto hot = slider.isMouseButtonDown();
        auto vf = display(21.0f, 0.02f);
        auto uf = mono(9.0f);
        auto nw = numberWidth(num, vf);
        auto uw = unit.isEmpty() ? 0.0f : juce::GlyphArrangement::getStringWidth(uf, unit) + 3.0f;
        auto row = r.removeFromTop(24).toFloat();
        auto x = row.getCentreX() - (nw + uw) * 0.5f;
        g.setColour(hot ? colour::accent : colour::text);
        drawNumber(g, num, vf, x, row.getCentreY());
        g.setColour(colour::text2);
        g.setFont(uf);
        g.drawText(unit, juce::Rectangle<float>(x + nw + 3, row.getY() + 2, uw + 2, row.getHeight()), juce::Justification::centredLeft, false);
        g.setFont(label(9.5f));
        g.drawFittedText(text, r.reduced(2, 0), juce::Justification::centredTop, 2, 0.9f);
    }

    juce::RangedAudioParameter& param;
    juce::String text;
    juce::Slider slider;
    juce::SliderParameterAttachment attach;
};

struct Segmented : juce::Component
{
    Segmented(juce::RangedAudioParameter& p, juce::String caption, bool compactPill = false)
        : param(p), options(p.getAllValueStrings()), text(caption), compact(compactPill),
          attach(p, [this](float v) { index = juce::roundToInt(v); repaint(); })
    {
        isSwitch = options.size() == 2 && options[0] == "Off" && options[1] == "On";
        attach.sendInitialUpdate();
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
        if (isDropdown())  // too many options for pills
        {
            combo.addItemList(options, 1);
            comboAttach = std::make_unique<juce::ComboBoxParameterAttachment>(p, combo);
            addAndMakeVisible(combo);
        }
    }
    bool isDropdown() const { return options.size() > 4; }
    void resized() override { combo.setBounds(pill().toNearestInt()); }
    static constexpr int pillH = 28, capH = 18;
    int top() const { return compact ? 0 : capH; }
    int height() const { return top() + (compact ? 22 : pillH); }
    float fontH() const { return compact ? 9.0f : 10.0f; }
    float segW(int i) const { return juce::GlyphArrangement::getStringWidth(label(fontH()), options[i].toUpperCase()) + (compact ? 16 : 22); }
    int preferredWidth() const
    {
        float w = 0; for (int i = 0; i < options.size(); ++i) w = isDropdown() ? juce::jmax(w, segW(i) + 34) : w + segW(i);
        return juce::jmax((int) std::ceil(w) + 2,
                          juce::GlyphArrangement::getStringWidthInt(label(), text.toUpperCase()) + 4);
    }
    juce::Rectangle<float> pill() const { return { 0.5f, (float) top() + 0.5f, preferredWidth() - 1.0f, (float) (height() - top()) - 1.0f }; }

    int hit(float x) const
    {
        float acc = 1;
        for (int i = 0; i < options.size(); ++i) { acc += segW(i); if (x < acc) return i; }
        return options.size() - 1;
    }
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (!isDropdown() && e.y >= top()) attach.setValueAsCompleteGesture((float) hit((float) e.x));
    }
    void mouseMove(const juce::MouseEvent& e) override { auto h = e.y >= top() ? hit((float) e.x) : -1; if (h != hover) { hover = h; repaint(); } }
    void mouseExit(const juce::MouseEvent&) override { hover = -1; repaint(); }

    void paint(juce::Graphics& g) override
    {
        if (!compact) paintCaption(g, text, getLocalBounds().removeFromTop(capH - 4));
        if (isDropdown()) return;
        auto p = pill();
        auto rad = p.getHeight() * 0.5f;
        g.setColour(colour::line);
        g.drawRoundedRectangle(p, rad, stroke::hair);
        float x = p.getX() + 0.5f;
        for (int i = 0; i < options.size(); ++i)
        {
            auto seg = juce::Rectangle<float>(x, p.getY(), segW(i), p.getHeight()).reduced(2.5f);
            auto sel = i == index;
            if (sel || i == hover)
            {
                g.setColour(sel ? (isSwitch && index == 1 ? colour::accent : colour::text) : colour::subtle);
                g.fillRoundedRectangle(seg, seg.getHeight() * 0.5f);
            }
            g.setColour(sel ? colour::card : colour::text);
            g.setFont(label(fontH()));
            g.drawText(options[i].toUpperCase(), seg, juce::Justification::centred, false);
            x += segW(i);
        }
    }

    juce::RangedAudioParameter& param;
    juce::StringArray options;
    juce::String text;
    bool compact;
    juce::ParameterAttachment attach;
    juce::ComboBox combo;
    std::unique_ptr<juce::ComboBoxParameterAttachment> comboAttach;
    int index = 0, hover = -1;
    bool isSwitch = false;
};

// Name -> header type / one-line description. Unknown plugins just show their name.
inline std::pair<juce::String, juce::String> pluginInfo(const juce::String& name)
{
    static const std::map<juce::String, std::pair<juce::String, juce::String>> m {
        { "PRISM",   { "EQ",     "6-band parametric equaliser" } },
        { "SQUEEZE", { "COMP",   "feed-forward compressor" } },
        { "GLOW",    { "TUBE",   "tube saturation" } },
        { "FLUX",    { "DYN EQ", "dynamic equaliser band" } },
        { "WALL",    { "LIMIT",  "brickwall limiter" } },
        { "GATE",    { "GATE",   "noise gate" } },
        { "SPACE",   { "VERB",   "algorithmic reverb" } },
        { "ECHO",    { "DELAY",  "stereo delay" } },
        { "SWIRL",   { "CHORUS", "stereo chorus" } },
        { "JET",     { "FLANGER", "through-zero style flanger" } },
        { "ORBIT",   { "PHASER", "multi-stage phaser" } },
        { "SWAY",    { "PAN",    "auto-pan / tremolo" } },
        { "ROOM",    { "IR",     "convolution reverb" } },
        { "CRUSH",   { "BITS",   "bitcrusher / downsampler" } },
        { "TUNE",    { "PITCH",  "pitch correction" } },
        { "TIDE",    { "WAVETABLE", "16-voice wavetable synth" } },
    };
    auto it = m.find(name);
    return it != m.end() ? it->second : std::pair<juce::String, juce::String>{};
}

// Group key for a parameter: explicit table for plugins with terse ids, else the id prefix before '_'.
// Empty key = ungrouped ("Controls", or "Master" when other groups exist).
inline juce::String groupKey(const juce::String& plugin, const juce::String& id)
{
    if (plugin == "TIDE")
    {
        static const std::map<juce::String, juce::String> t {
            { "uni", "Voice" }, { "detune", "Voice" }, { "spread", "Voice" },
            { "ftype", "Filter" }, { "cut", "Filter" }, { "res", "Filter" }, { "fenv", "Filter" }, { "ktrk", "Filter" },
            { "a", "Amp Env" }, { "d", "Amp Env" }, { "s", "Amp Env" }, { "r", "Amp Env" },
            { "fa", "Filter Env" }, { "fd", "Filter Env" }, { "fs", "Filter Env" }, { "fr", "Filter Env" },
            { "lrate", "LFO" }, { "lpos", "LFO" }, { "lcut", "LFO" },
        };
        if (auto it = t.find(id); it != t.end()) return it->second;
    }
    return id.containsChar('_') ? id.upToFirstOccurrenceOf("_", false, false) : juce::String();
}
inline juce::String firstWord(const juce::String& name) { return name.upToFirstOccurrenceOf(" ", false, false); }

// Leading words shared by a and b ("Bell 1 Freq", "Bell 1 Q" -> "Bell 1").
inline juce::String commonWords(const juce::String& a, const juce::String& b)
{
    auto x = juce::StringArray::fromTokens(a, " ", ""), y = juce::StringArray::fromTokens(b, " ", "");
    juce::StringArray out;
    for (int i = 0; i < juce::jmin(x.size(), y.size()) && x[i] == y[i]; ++i) out.add(x[i]);
    return out.joinIntoString(" ");
}

// Magnitude response card (log 20 Hz - 20 kHz, +-18 dB), polled from the processor's HasResponse hook.
struct ResponseView : juce::Component, juce::Timer
{
    static constexpr int N = 256;
    explicit ResponseView(const HasResponse& r) : src(r)
    {
        for (int i = 0; i < N; ++i) hz[i] = 20.0 * std::pow(1000.0, i / (N - 1.0));
        startTimerHz(30);
        timerCallback();
    }
    void timerCallback() override
    {
        float next[N];
        src.responseDb(hz, next, N);
        if (std::equal(next, next + N, db)) return;
        std::copy(next, next + N, db);
        repaint();
    }
    juce::Rectangle<float> plot() const { return getLocalBounds().toFloat().reduced(44, 0).withTrimmedTop(34).withTrimmedBottom(28); }
    float xOf(double f) const { auto p = plot(); return p.getX() + p.getWidth() * (float) (std::log(f / 20.0) / std::log(1000.0)); }
    float yOf(float d) const { auto p = plot(); return p.getCentreY() - p.getHeight() * 0.5f * juce::jlimit(-1.1f, 1.1f, d / 18.0f); }

    void paint(juce::Graphics& g) override
    {
        paintCard(g, getLocalBounds().toFloat());
        auto cap = getLocalBounds().reduced(16, 0).withTop(12).withHeight(14);
        g.setColour(colour::text); g.setFont(label(10.0f));
        g.drawText("RESPONSE", cap, juce::Justification::centredLeft, false);
        paintCaption(g, "20 HZ - 20 KHZ", cap, juce::Justification::centredRight);

        auto p = plot();
        auto dots = [&](float x0, float y0, float x1, float y1, juce::Colour c)
        {
            g.setColour(c);
            auto len = std::hypot(x1 - x0, y1 - y0);
            for (float t = 0; t <= len; t += 4.0f)
                g.fillRect(x0 + (x1 - x0) * t / len - 0.6f, y0 + (y1 - y0) * t / len - 0.6f, 1.2f, 1.2f);
        };
        g.setFont(label(9.0f));
        for (int d = -18; d <= 18; d += 6)
        {
            dots(p.getX(), yOf((float) d), p.getRight(), yOf((float) d), d == 0 ? colour::text2 : colour::line);
            if (d % 12 == 0)
            {
                g.setColour(colour::text2);
                g.drawText((d > 0 ? "+" : "") + juce::String(d), juce::Rectangle<float>(p.getX() - 40, yOf((float) d) - 7, 32, 14),
                           juce::Justification::centredRight, false);
            }
        }
        for (double f : { 50.0, 100.0, 200.0, 500.0, 1000.0, 2000.0, 5000.0, 10000.0 })
        {
            dots(xOf(f), p.getY(), xOf(f), p.getBottom(), colour::line);
            g.setColour(colour::text2);
            g.drawText(f >= 1000 ? juce::String((int) f / 1000) + "K" : juce::String((int) f),
                       juce::Rectangle<float>(xOf(f) - 20, p.getBottom() + 6, 40, 14), juce::Justification::centred, false);
        }
        juce::Path curve;
        for (int i = 0; i < N; ++i)
        {
            auto pt = juce::Point<float>(xOf(hz[i]), yOf(db[i]));
            if (i == 0) curve.startNewSubPath(pt); else curve.lineTo(pt);
        }
        auto fill = curve;
        fill.lineTo(p.getRight(), yOf(0)); fill.lineTo(p.getX(), yOf(0)); fill.closeSubPath();
        g.setColour(colour::text.withAlpha(0.05f)); g.fillPath(fill);
        g.setColour(colour::text);
        g.strokePath(curve, juce::PathStrokeType(1.75f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    const HasResponse& src;
    double hz[N];
    float db[N] {};
};

// Content is laid out once at design size and scaled as a whole on resize.
struct Panel : juce::Component
{
    static constexpr int margin = 16, gap = 12, headerH = 80, cellW = 92, cellH = 124, pad = 14, capH = 34;

    struct Group
    {
        juce::String key, title;
        std::vector<juce::RangedAudioParameter*> params;
        std::vector<Knob*> knobs;
        std::vector<Segmented*> choices;
        Segmented* headerSwitch = nullptr;
        juce::Rectangle<int> bounds;
        int cols = 1, rows = 0, natH = 0;
    };

    explicit Panel(juce::AudioProcessor& p)
    {
        name = p.getName().fromFirstOccurrenceOf("SW ", false, false).toUpperCase();
        if (name.isEmpty()) name = p.getName().toUpperCase();
        info = pluginInfo(name);

        int count = 0;
        for (auto* ap : p.getParameters())
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(ap))
            {
                auto key = groupKey(name, rp->getParameterID());
                if (key.isEmpty())  // names sharing a first word form a section ("Feedback", "Feedback Low Cut")
                {
                    int same = 0;
                    for (auto* o : p.getParameters())
                        if (auto* orp = dynamic_cast<juce::RangedAudioParameter*>(o))
                            same += groupKey(name, orp->getParameterID()).isEmpty() && firstWord(orp->getName(64)) == firstWord(rp->getName(64));
                    if (same > 1) key = firstWord(rp->getName(64));
                }
                auto it = std::find_if(groups.begin(), groups.end(), [&](auto& g) { return g.key == key; });
                if (it == groups.end()) { groups.emplace_back(); groups.back().key = key; it = groups.end() - 1; }
                it->params.push_back(rp);
                ++count;
            }
        W = count > 12 ? 960 : 640;  // big plugins (EQ, synth) get a wider default

        for (auto& g : groups)
        {
            if (g.key.isEmpty()) g.title = groups.size() > 1 ? "Main" : "Controls";
            else
            {
                g.title = g.key;
                if (!g.key.containsChar(' '))  // derive a readable title from the shared words ("lc" -> "Low Cut")
                {
                    g.title = g.params[0]->getName(64);
                    for (auto* rp : g.params) g.title = commonWords(g.title, rp->getName(64));
                }
                if (g.title.isEmpty()) g.title = g.key;
            }
            for (auto* rp : g.params)
            {
                // caption = name minus the group title's leading words
                auto full = rp->getName(64);
                auto pre = g.key.isEmpty() ? juce::String() : commonWords(g.title, full);
                auto cap = full.substring(pre.length()).trim();
                if (dynamic_cast<juce::AudioParameterChoice*>(rp))
                {
                    auto* s = new Segmented(*rp, cap.isEmpty() ? full : cap, cap.isEmpty() && rp->getAllValueStrings().size() == 2);
                    addAndMakeVisible(choiceStore.add(s));
                    if (s->compact) g.headerSwitch = s; else g.choices.push_back(s);
                }
                else addAndMakeVisible(g.knobs.emplace_back(knobStore.add(new Knob(*rp, cap.isEmpty() ? juce::String("Amount") : cap))));
            }
        }
        if (auto* r = dynamic_cast<HasResponse*>(&p))
            addAndMakeVisible(*(response = std::make_unique<ResponseView>(*r)));
        if ((loader = dynamic_cast<LoadsFile*>(&p)) != nullptr)
        {
            loadButton.setButtonText("Load IR");
            loadButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
            loadButton.onClick = [this, &p] { chooseFile(p); };
            addAndMakeVisible(loadButton);
        }
        layout();
    }

    void chooseFile(juce::AudioProcessor& p)
    {
        chooser = std::make_unique<juce::FileChooser>("Load impulse response", juce::File(), "*.wav;*.aif;*.aiff");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, &p](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (!f.existsAsFile()) return;
                loader->loadImpulseFile(f);
                loadButton.setButtonText(f.getFileNameWithoutExtension());
                for (auto* ap : p.getParameters())
                    if (auto* c = dynamic_cast<juce::AudioParameterChoice*>(ap))
                        if (auto i = c->choices.indexOf("Custom File"); i >= 0)
                        {
                            c->beginChangeGesture();
                            c->setValueNotifyingHost(c->convertTo0to1((float) i));
                            c->endChangeGesture();
                        }
            });
    }

    void layout()
    {
        const int avail = W - 2 * margin, inner = avail - 2 * pad, chRow = Segmented::capH + Segmented::pillH + 14;
        // natural size of each group card
        for (auto& g : groups)
        {
            int n = (int) g.knobs.size(), maxCols = juce::jmax(1, inner / cellW);
            g.rows = n ? (n + maxCols - 1) / maxCols : 0;
            g.cols = n ? (n + g.rows - 1) / g.rows : 0;
            int cw = 0, chRows = g.choices.empty() ? 0 : 1, x = 0;
            for (auto* c : g.choices)
            {
                auto w = c->preferredWidth();
                if (x > 0 && x + w > inner) { x = 0; ++chRows; }
                x += w + 24; cw = juce::jmax(cw, x - 24);
            }
            auto head = juce::GlyphArrangement::getStringWidthInt(label(), g.title.toUpperCase()) + 24
                        + (g.headerSwitch ? g.headerSwitch->preferredWidth() : 24);
            int w = juce::jmin(avail, juce::jmax(g.cols * cellW, cw, head) + 2 * pad);
            g.bounds = { 0, 0, w, capH + chRows * chRow + g.rows * cellH + 6 };
            g.natH = g.bounds.getHeight();
        }
        if (loader && !groups.empty() && !groups[0].headerSwitch)
            groups[0].bounds.setWidth(juce::jmax(groups[0].bounds.getWidth(), 260));
        // greedy rows, cards stretched to fill each row
        int y = headerH;
        if (response) { response->setBounds(margin, y, avail, 180); y += 180 + gap; }
        for (size_t i = 0; i < groups.size();)
        {
            size_t j = i; int used = 0;
            while (j < groups.size() && (j == i || used + gap + groups[j].bounds.getWidth() <= avail))
            {
                used += (j == i ? 0 : gap) + groups[j].bounds.getWidth();
                ++j;
            }
            int extra = avail - used, rowH = 0, x = margin, natural = used - gap * (int) (j - i - 1);
            for (size_t k = i; k < j; ++k) rowH = juce::jmax(rowH, groups[k].bounds.getHeight());
            for (size_t k = i; k < j; ++k)
            {
                auto& g = groups[k];
                int w = g.bounds.getWidth() + (k + 1 == j ? avail - (x - margin) - g.bounds.getWidth()
                                                          : extra * g.bounds.getWidth() / natural);
                g.bounds = { x, y, w, rowH };
                place(g);
                x += w + gap;
            }
            y += rowH + gap;
            i = j;
        }
        if (loader && !groups.empty() && !groups[0].headerSwitch)
            loadButton.setBounds(groups[0].bounds.getRight() - pad - 140 + 4, groups[0].bounds.getY() + 8, 140, 22);
        setSize(W, y - gap + margin + 4);
    }

    void place(Group& g)
    {
        auto r = g.bounds;
        if (g.headerSwitch)
        {
            auto w = g.headerSwitch->preferredWidth();
            g.headerSwitch->setBounds(r.getRight() - pad - w + 4, r.getY() + 8, w, g.headerSwitch->height());
        }
        int y = r.getY() + capH + (r.getHeight() - g.natH) / 2, x = 0, inner = r.getWidth() - 2 * pad;
        for (auto* c : g.choices)
        {
            auto w = c->preferredWidth();
            if (x > 0 && x + w > inner) { x = 0; y += Segmented::capH + Segmented::pillH + 14; }
            c->setBounds(r.getX() + pad + x, y, w, c->height());
            x += w + 24;
        }
        if (!g.choices.empty()) y += Segmented::capH + Segmented::pillH + 14;
        const int n = (int) g.knobs.size();
        const int cgap = g.cols > 1 ? juce::jmin(24, (inner - g.cols * cellW) / (g.cols - 1)) : 0;
        for (int i = 0; i < n; ++i)
        {
            int row = i / g.cols, c = i % g.cols, inRow = juce::jmin(g.cols, n - row * g.cols);
            int rowW = inRow * cellW + (inRow - 1) * cgap;
            g.knobs[(size_t) i]->setBounds(r.getX() + (r.getWidth() - rowW) / 2 + c * (cellW + cgap), y + row * cellH, cellW, cellH);
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto h = juce::Rectangle<int>(margin + 6, 18, W - 2 * margin - 12, 36);
        auto title = name + (info.first.isNotEmpty() ? " ( " + info.first + " )" : juce::String());
        g.setColour(colour::text);
        g.setFont(display(36.0f, 0.04f));
        g.drawText(title, h, juce::Justification::centredLeft, false);
        g.setFont(mono(10.0f, true, 0.3f));
        g.drawText("SOUNDWAVE", h.withHeight(16), juce::Justification::topRight, false);
        g.setColour(colour::text2);
        g.setFont(label(10.0f));
        g.drawText(info.second.toUpperCase(), h.translated(0, 36).withHeight(16), juce::Justification::centredLeft, false);
        g.drawText("V" + juce::String(JucePlugin_VersionString), h.withTrimmedTop(18).withHeight(16), juce::Justification::topRight, false);

        int idx = 0;
        for (auto& gr : groups)
        {
            paintCard(g, gr.bounds.toFloat());
            auto c = gr.bounds.reduced(pad + 2, 0).withTop(gr.bounds.getY() + 12).withHeight(14);
            g.setColour(colour::text);
            g.setFont(label(10.0f));
            g.drawText(gr.title.toUpperCase(), c, juce::Justification::centredLeft, true);
            if (!gr.headerSwitch && !(loader && &gr == &groups[0]))
                paintCaption(g, juce::String(++idx).paddedLeft('0', 2), c, juce::Justification::centredRight);
            else ++idx;
        }
    }

    int W = 640;
    juce::String name;
    std::pair<juce::String, juce::String> info;
    std::vector<Group> groups;
    juce::OwnedArray<Knob> knobStore;
    juce::OwnedArray<Segmented> choiceStore;
    LoadsFile* loader = nullptr;
    std::unique_ptr<ResponseView> response;
    juce::TextButton loadButton;
    std::unique_ptr<juce::FileChooser> chooser;
};

struct Editor : juce::AudioProcessorEditor
{
    explicit Editor(juce::AudioProcessor& p) : juce::AudioProcessorEditor(p), panel(p)
    {
        setLookAndFeel(&look);
        addAndMakeVisible(panel);
        const int w = panel.getWidth(), h = panel.getHeight();
        setResizable(true, true);
        setResizeLimits(w * 3 / 4, h * 3 / 4, w * 2, h * 2);
        getConstrainer()->setFixedAspectRatio((double) w / h);
        setSize(w, h);
    }
    ~Editor() override { setLookAndFeel(nullptr); }

    void paint(juce::Graphics& g) override { paintDotGrid(g, getLocalBounds()); }
    void resized() override
    {
        panel.setTransform(juce::AffineTransform::scale((float) getWidth() / (float) panel.getWidth()));
    }

    LookAndFeel look;  // declared first: outlives panel
    Panel panel;
};
} // namespace sw
