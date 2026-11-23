#pragma once
// SoundWave design system: tokens, fonts, painting helpers and LookAndFeel.
// Standalone (juce_gui_basics only) so plugins and the future DAW share one look.
#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"   // SWFonts (juce_add_binary_data)

namespace sw
{
// ---- tokens ---------------------------------------------------------------
namespace colour
{
    inline const juce::Colour bg       { 0xffefefef };  // window background
    inline const juce::Colour dot      { 0xffcfcfcf };  // dot grid
    inline const juce::Colour card     { 0xffffffff };
    inline const juce::Colour line     { 0xffdadada };  // hairlines, tracks
    inline const juce::Colour subtle   { 0xfff4f4f4 };  // hover fills
    inline const juce::Colour text     { 0xff111111 };
    inline const juce::Colour text2    { 0xff8a8a8a };  // secondary
    inline const juce::Colour accent   { 0xffd71921 };  // ONLY for "active" state
}
namespace space  { constexpr int xs = 4, s = 8, m = 12, l = 16, xl = 24, grid = 16; }
namespace radius { constexpr float card = 18.0f, control = 10.0f, pill = 1000.0f; }
namespace stroke { constexpr float hair = 1.0f, track = 2.0f, value = 2.5f; }

// ---- fonts ----------------------------------------------------------------
inline juce::Typeface::Ptr dotoFace()
{
    static auto t = juce::Typeface::createSystemTypefaceFor(BinaryData::Doto_ttf, BinaryData::Doto_ttfSize);
    return t;
}
inline juce::Typeface::Ptr monoFace(bool bold = false)
{
    static auto r = juce::Typeface::createSystemTypefaceFor(BinaryData::SpaceMonoRegular_ttf, BinaryData::SpaceMonoRegular_ttfSize);
    static auto b = juce::Typeface::createSystemTypefaceFor(BinaryData::SpaceMonoBold_ttf, BinaryData::SpaceMonoBold_ttfSize);
    return bold ? b : r;
}
// Display / numeric readouts (dot matrix)
inline juce::Font display(float h, float tracking = 0.04f)
{
    return juce::Font(juce::FontOptions(dotoFace()).withHeight(h).withKerningFactor(tracking));
}
// Body / labels (mono). Labels are usually uppercase + letterspaced.
inline juce::Font mono(float h, bool bold = false, float tracking = 0.0f)
{
    return juce::Font(juce::FontOptions(monoFace(bold)).withHeight(h).withKerningFactor(tracking));
}
inline juce::Font label(float h = 10.0f) { return mono(h, false, 0.12f); }

// Numbers in the display font. Doto's '.' reads like '+', so the point is drawn as a square dot.
inline float numberWidth(const juce::String& s, const juce::Font& f)
{
    auto runs = juce::StringArray::fromTokens(s, ".", "");
    float w = (runs.size() - 1) * f.getHeight() * 0.3f;
    for (auto& r : runs) w += juce::GlyphArrangement::getStringWidth(f, r);
    return w;
}
// Draws s with its left edge at x and the digits vertically centred on centreY.
inline void drawNumber(juce::Graphics& g, const juce::String& s, const juce::Font& f, float x, float centreY)
{
    g.setFont(f);
    auto h = f.getHeight(), base = centreY + f.getAscent() * 0.36f, dot = h * 0.11f;
    auto runs = juce::StringArray::fromTokens(s, ".", "");
    for (int i = 0; i < runs.size(); ++i)
    {
        if (i > 0) { g.fillRect(x + h * 0.15f - dot * 0.5f, base - dot, dot, dot); x += h * 0.3f; }
        g.drawSingleLineText(runs[i], juce::roundToInt(x), juce::roundToInt(base));
        x += juce::GlyphArrangement::getStringWidth(f, runs[i]);
    }
}

// ---- painting helpers -----------------------------------------------------
inline void paintDotGrid(juce::Graphics& g, juce::Rectangle<int> area, int step = space::grid)
{
    g.setColour(colour::bg);
    g.fillRect(area);
    g.setColour(colour::dot);
    for (int y = area.getY() + step / 2; y < area.getBottom(); y += step)
        for (int x = area.getX() + step / 2; x < area.getRight(); x += step)
            g.fillEllipse((float) x - 0.9f, (float) y - 0.9f, 1.8f, 1.8f);
}
inline void paintCard(juce::Graphics& g, juce::Rectangle<float> r, float rad = radius::card)
{
    g.setColour(colour::card);
    g.fillRoundedRectangle(r, rad);
    g.setColour(colour::line);
    g.drawRoundedRectangle(r.reduced(0.5f), rad, stroke::hair);
}
// Small uppercase caption, e.g. card titles.
inline void paintCaption(juce::Graphics& g, const juce::String& s, juce::Rectangle<int> r,
                         juce::Justification j = juce::Justification::centredLeft, juce::Colour c = colour::text2)
{
    g.setColour(c);
    g.setFont(label());
    g.drawText(s.toUpperCase(), r, j, true);
}

// ---- LookAndFeel ----------------------------------------------------------
struct LookAndFeel : juce::LookAndFeel_V4
{
    LookAndFeel() : juce::LookAndFeel_V4(juce::LookAndFeel_V4::getLightColourScheme())
    {
        using namespace colour;
        auto s = getCurrentColourScheme();
        s.setUIColour(ColourScheme::windowBackground, bg);
        s.setUIColour(ColourScheme::widgetBackground, card);
        s.setUIColour(ColourScheme::menuBackground,   card);
        s.setUIColour(ColourScheme::outline,          line);
        s.setUIColour(ColourScheme::defaultText,      text);
        s.setUIColour(ColourScheme::defaultFill,      text);
        s.setUIColour(ColourScheme::highlightedText,  card);
        s.setUIColour(ColourScheme::highlightedFill,  text);
        s.setUIColour(ColourScheme::menuText,         text);
        setColourScheme(s);

        setColour(juce::ResizableWindow::backgroundColourId, bg);
        setColour(juce::Label::textColourId, text);
        setColour(juce::TextButton::buttonColourId, card);
        setColour(juce::TextButton::buttonOnColourId, text);
        setColour(juce::TextButton::textColourOffId, text);
        setColour(juce::TextButton::textColourOnId, card);
        setColour(juce::ComboBox::backgroundColourId, card);
        setColour(juce::ComboBox::outlineColourId, line);
        setColour(juce::ComboBox::textColourId, text);
        setColour(juce::ComboBox::arrowColourId, text);
        setColour(juce::PopupMenu::backgroundColourId, card);
        setColour(juce::PopupMenu::textColourId, text);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, subtle);
        setColour(juce::PopupMenu::highlightedTextColourId, text);
        setColour(juce::TextEditor::backgroundColourId, card);
        setColour(juce::TextEditor::outlineColourId, line);
        setColour(juce::TextEditor::focusedOutlineColourId, text);
        setColour(juce::TextEditor::textColourId, text);
        setColour(juce::TextEditor::highlightColourId, line);
        setColour(juce::CaretComponent::caretColourId, accent);
        setColour(juce::Slider::textBoxTextColourId, text);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::ScrollBar::thumbColourId, text2);
        setColour(juce::TooltipWindow::backgroundColourId, text);
        setColour(juce::TooltipWindow::textColourId, card);
        setColour(juce::TabbedComponent::backgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::TabbedComponent::outlineColourId, juce::Colours::transparentBlack);
        setColour(juce::TabbedButtonBar::tabOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::TabbedButtonBar::frontOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::ListBox::backgroundColourId, card);
    }

    // Every default-font text in JUCE becomes Space Mono.
    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& f) override
    {
        if (f.getTypefaceName() == juce::Font::getDefaultSansSerifFontName()
            || f.getTypefaceName() == juce::Font::getDefaultMonospacedFontName())
            return monoFace(f.isBold());
        return juce::LookAndFeel_V4::getTypefaceForFont(f);
    }

    // Active = being dragged or hovered.
    static bool isActive(juce::Component& c) { return c.isMouseButtonDown() || c.isMouseOverOrDragging(); }

    // --- rotary knob: thin track, value arc, dot indicator
    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos,
                          float a0, float a1, juce::Slider& s) override
    {
        auto b = juce::Rectangle<int>(x, y, w, h).toFloat();
        auto size = juce::jmin(b.getWidth(), b.getHeight());
        auto c = b.getCentre();
        auto r = size * 0.5f - 3.0f;
        auto ang = a0 + pos * (a1 - a0);
        auto hot = s.isMouseButtonDown() || s.isMouseOverOrDragging();
        auto ink = hot ? colour::accent : colour::text;

        juce::Path track;
        track.addCentredArc(c.x, c.y, r, r, 0, a0, a1, true);
        g.setColour(colour::line);
        g.strokePath(track, juce::PathStrokeType(stroke::track, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // bipolar ranges (e.g. -18..+18 dB) draw from zero
        auto from = a0;
        if (s.getMinimum() < 0.0 && s.getMaximum() > 0.0)
            from = a0 + (float) s.valueToProportionOfLength(0.0) * (a1 - a0);
        if (std::abs(ang - from) > 0.001f)
        {
            juce::Path arc;
            arc.addCentredArc(c.x, c.y, r, r, 0, juce::jmin(from, ang), juce::jmax(from, ang), true);
            g.setColour(ink);
            g.strokePath(arc, juce::PathStrokeType(stroke::value, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // face: white disc with hairline, dot indicator
        auto fr = r * 0.70f;
        g.setColour(hot ? colour::subtle : colour::card);
        g.fillEllipse(c.x - fr, c.y - fr, fr * 2, fr * 2);
        g.setColour(colour::line);
        g.drawEllipse(c.x - fr, c.y - fr, fr * 2, fr * 2, stroke::hair);
        auto dp = c.getPointOnCircumference(fr * 0.66f, ang);
        auto dr = juce::jmax(2.0f, size * 0.035f);
        g.setColour(ink);
        g.fillEllipse(dp.x - dr, dp.y - dr, dr * 2, dr * 2);
    }

    // --- linear slider / fader: hairline track, black fill, pill cap
    void drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float minPos, float maxPos,
                          juce::Slider::SliderStyle style, juce::Slider& s) override
    {
        if (style != juce::Slider::LinearHorizontal && style != juce::Slider::LinearVertical
            && style != juce::Slider::LinearBar && style != juce::Slider::LinearBarVertical)
            return juce::LookAndFeel_V4::drawLinearSlider(g, x, y, w, h, pos, minPos, maxPos, style, s);

        auto b = juce::Rectangle<int>(x, y, w, h).toFloat();
        auto vert = s.isVertical();
        auto hot = isActive(s);
        if (style == juce::Slider::LinearBar || style == juce::Slider::LinearBarVertical)
        {
            g.setColour(colour::subtle); g.fillRoundedRectangle(b, radius::control * 0.6f);
            auto f = vert ? b.withTop(pos) : b.withRight(pos);
            g.setColour(hot ? colour::accent : colour::text); g.fillRoundedRectangle(f, radius::control * 0.6f);
            return;
        }
        auto c = b.getCentre();
        auto start = vert ? juce::Point<float>(c.x, b.getBottom()) : juce::Point<float>(b.getX(), c.y);
        auto thumb = vert ? juce::Point<float>(c.x, pos) : juce::Point<float>(pos, c.y);
        g.setColour(colour::line);
        g.drawLine(juce::Line<float>(start, vert ? juce::Point<float>(c.x, b.getY()) : juce::Point<float>(b.getRight(), c.y)), stroke::track);
        g.setColour(hot ? colour::accent : colour::text);
        g.drawLine(juce::Line<float>(start, thumb), stroke::value);
        // fader cap
        auto cap = vert ? juce::Rectangle<float>(0, 0, juce::jmin(28.0f, b.getWidth()), 12.0f)
                        : juce::Rectangle<float>(0, 0, 12.0f, juce::jmin(28.0f, b.getHeight()));
        cap = cap.withCentre(thumb);
        g.setColour(colour::card); g.fillRoundedRectangle(cap, 6.0f);
        g.setColour(hot ? colour::accent : colour::text); g.drawRoundedRectangle(cap.reduced(0.75f), 6.0f, 1.5f);
        g.drawLine(vert ? juce::Line<float>(cap.getX() + 6, thumb.y, cap.getRight() - 6, thumb.y)
                        : juce::Line<float>(thumb.x, cap.getY() + 6, thumb.x, cap.getBottom() - 6), 1.0f);
    }
    int getSliderThumbRadius(juce::Slider& s) override { return s.isRotary() ? 0 : 7; }

    // --- buttons: pills; toggled-on = black, toggled-on + accent flag = red
    void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override
    {
        auto r = b.getLocalBounds().toFloat().reduced(0.5f);
        auto rad = r.getHeight() * 0.5f;
        auto on = b.getToggleState();
        auto fill = on ? (b.getProperties()["swAccent"] ? colour::accent : colour::text)
                       : (down ? colour::line : over ? colour::subtle : colour::card);
        g.setColour(fill); g.fillRoundedRectangle(r, rad);
        if (!on) { g.setColour(colour::line); g.drawRoundedRectangle(r, rad, stroke::hair); }
    }
    juce::Font getTextButtonFont(juce::TextButton&, int h) override { return label(juce::jlimit(9.0f, 13.0f, h * 0.4f)); }
    void drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) override
    {
        g.setFont(getTextButtonFont(b, b.getHeight()));
        g.setColour((b.getToggleState() ? colour::card : colour::text).withMultipliedAlpha(b.isEnabled() ? 1.0f : 0.4f));
        g.drawText(b.getButtonText().toUpperCase(), b.getLocalBounds().reduced(6, 0), juce::Justification::centred, true);
    }

    // --- toggle: small switch (red when on) + label
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool over, bool) override
    {
        auto r = b.getLocalBounds().toFloat();
        auto sw = juce::Rectangle<float>(r.getX() + 2, r.getCentreY() - 8, 30, 16);
        auto on = b.getToggleState();
        g.setColour(on ? colour::accent : (over ? colour::subtle : colour::card));
        g.fillRoundedRectangle(sw, 8);
        if (!on) { g.setColour(colour::line); g.drawRoundedRectangle(sw.reduced(0.5f), 8, stroke::hair); }
        auto k = juce::Rectangle<float>(12, 12).withCentre({ on ? sw.getRight() - 8 : sw.getX() + 8, sw.getCentreY() });
        g.setColour(on ? colour::card : colour::text); g.fillEllipse(k);
        g.setColour(colour::text.withMultipliedAlpha(b.isEnabled() ? 1.0f : 0.4f));
        g.setFont(label(11.0f));
        g.drawText(b.getButtonText().toUpperCase(), r.withLeft(sw.getRight() + 8), juce::Justification::centredLeft, true);
    }

    // --- combo box / popup menus
    void drawComboBox(juce::Graphics& g, int w, int h, bool down, int, int, int, int, juce::ComboBox& cb) override
    {
        auto r = juce::Rectangle<float>(0, 0, (float) w, (float) h).reduced(0.5f);
        g.setColour(down || cb.isPopupActive() ? colour::subtle : colour::card);
        g.fillRoundedRectangle(r, r.getHeight() * 0.5f);
        g.setColour(cb.hasKeyboardFocus(true) || cb.isPopupActive() ? colour::text : colour::line);
        g.drawRoundedRectangle(r, r.getHeight() * 0.5f, stroke::hair);
        juce::Path p;  // chevron
        auto cx = (float) w - h * 0.55f, cy = h * 0.5f;
        p.startNewSubPath(cx - 4, cy - 2); p.lineTo(cx, cy + 2); p.lineTo(cx + 4, cy - 2);
        g.setColour(colour::text); g.strokePath(p, juce::PathStrokeType(1.5f));
    }
    juce::Font getComboBoxFont(juce::ComboBox& cb) override { return mono(juce::jlimit(10.0f, 14.0f, cb.getHeight() * 0.45f)); }
    void positionComboBoxText(juce::ComboBox& cb, juce::Label& l) override
    {
        l.setBounds(cb.getHeight() / 2, 0, cb.getWidth() - cb.getHeight() * 3 / 2, cb.getHeight());
        l.setFont(getComboBoxFont(cb));
    }
    void drawPopupMenuBackground(juce::Graphics& g, int w, int h) override
    {
        g.fillAll(colour::card);
        g.setColour(colour::line);
        g.drawRect(0, 0, w, h, 1);
    }
    juce::Font getPopupMenuFont() override { return mono(13.0f); }
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, bool sep, bool active, bool hi, bool ticked,
                           bool sub, const juce::String& text, const juce::String& shortcut, const juce::Drawable*, const juce::Colour*) override
    {
        if (sep) { g.setColour(colour::line); g.fillRect(area.reduced(8, 0).withHeight(1).withY(area.getCentreY())); return; }
        auto r = area.reduced(4, 1);
        if (hi && active) { g.setColour(colour::subtle); g.fillRoundedRectangle(r.toFloat(), 6.0f); }
        if (ticked) { g.setColour(colour::accent); g.fillEllipse(juce::Rectangle<float>(6, 6).withCentre({ r.getX() + 10.0f, (float) r.getCentreY() })); }
        g.setColour(colour::text.withMultipliedAlpha(active ? 1.0f : 0.4f));
        g.setFont(getPopupMenuFont());
        auto tr = r.withTrimmedLeft(20).withTrimmedRight(12);
        g.drawText(text, tr, juce::Justification::centredLeft, true);
        if (shortcut.isNotEmpty()) { g.setColour(colour::text2); g.drawText(shortcut, tr, juce::Justification::centredRight, true); }
        if (sub) g.drawText(">", tr, juce::Justification::centredRight, false);
    }

    // --- scrollbars: thin pill thumb, no track
    int getDefaultScrollbarWidth() override { return 8; }
    void drawScrollbar(juce::Graphics& g, juce::ScrollBar&, int x, int y, int w, int h, bool vert,
                       int start, int size, bool over, bool down) override
    {
        auto t = vert ? juce::Rectangle<int>(x, start, w, size) : juce::Rectangle<int>(start, y, size, h);
        g.setColour((down ? colour::text : over ? colour::text2 : colour::line));
        g.fillRoundedRectangle(t.toFloat().reduced(2.0f), (vert ? w : h) * 0.5f);
    }

    // --- tabs: segmented pills (front = black)
    int getTabButtonOverlap(int) override { return 0; }
    int getTabButtonBestWidth(juce::TabBarButton& b, int depth) override
    {
        return juce::GlyphArrangement::getStringWidthInt(label(11.0f), b.getButtonText().toUpperCase()) + depth;
    }
    void drawTabButton(juce::TabBarButton& b, juce::Graphics& g, bool over, bool) override
    {
        auto r = b.getActiveArea().toFloat().reduced(2.0f, 3.0f);
        auto front = b.isFrontTab();
        g.setColour(front ? colour::text : over ? colour::subtle : juce::Colours::transparentBlack);
        g.fillRoundedRectangle(r, r.getHeight() * 0.5f);
        g.setColour(front ? colour::card : colour::text);
        g.setFont(label(11.0f));
        g.drawText(b.getButtonText().toUpperCase(), r, juce::Justification::centred, true);
    }
    void drawTabbedButtonBarBackground(juce::TabbedButtonBar&, juce::Graphics&) override {}
    void drawTabAreaBehindFrontButton(juce::TabbedButtonBar&, juce::Graphics&, int, int) override {}

    void drawCornerResizer(juce::Graphics& g, int w, int h, bool over, bool down) override
    {
        g.setColour(down || over ? colour::text : colour::text2);
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j <= i; ++j)
                g.fillEllipse(w - 4.0f - j * 4.0f - 1, h - 4.0f - (i - j) * 4.0f - 1, 2, 2);
    }

    // --- tooltips / labels / text editors
    void drawTooltip(juce::Graphics& g, const juce::String& t, int w, int h) override
    {
        g.setColour(colour::text); g.fillRoundedRectangle(0, 0, (float) w, (float) h, 6.0f);
        g.setColour(colour::card); g.setFont(mono(12.0f));
        g.drawText(t, 0, 0, w, h, juce::Justification::centred, true);
    }
    juce::Font getLabelFont(juce::Label& l) override { return mono(juce::jmin(14.0f, l.getFont().getHeight())); }
    void drawTextEditorOutline(juce::Graphics& g, int w, int h, juce::TextEditor& e) override
    {
        g.setColour(e.hasKeyboardFocus(true) ? colour::text : colour::line);
        g.drawRoundedRectangle(0.5f, 0.5f, w - 1.0f, h - 1.0f, radius::control, stroke::hair);
    }
    void fillTextEditorBackground(juce::Graphics& g, int w, int h, juce::TextEditor&) override
    {
        g.setColour(colour::card); g.fillRoundedRectangle(0, 0, (float) w, (float) h, radius::control);
    }
};
} // namespace sw
