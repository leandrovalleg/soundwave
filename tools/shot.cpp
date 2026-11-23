// sw_shot <plugin.vst3> <out.png> [width]: shows the plugin editor at the X origin and grabs it with ImageMagick.
// The VST3 view is a native child window, so we grab the X framebuffer instead of createComponentSnapshot().
#include <juce_audio_processors/juce_audio_processors.h>
#include <X11/Xlib.h>

int main(int argc, char** argv)
{
    if (argc < 3) { printf("usage: sw_shot plugin.vst3 out.png [width]\n"); return 2; }
    // Bare Xvfb has no window manager, so JUCE's "if exists" WM atoms come back as 0 -> BadAtom. Create them.
    if (auto* d = XOpenDisplay(nullptr))
    {
        for (auto* a : { "WM_PROTOCOLS", "WM_TAKE_FOCUS", "WM_DELETE_WINDOW", "_NET_WM_PING", "WM_CHANGE_STATE", "WM_STATE",
                         "_NET_WM_WINDOW_TYPE", "_NET_WM_STATE", "_NET_WM_STATE_HIDDEN" })
            XInternAtom(d, a, False);
        XSync(d, False);
    }
    juce::ScopedJuceInitialiser_GUI init;
    juce::VST3PluginFormat fmt;
    juce::OwnedArray<juce::PluginDescription> found;
    fmt.findAllTypesForFile(found, argv[1]);
    if (found.isEmpty()) { printf("FAIL load\n"); return 1; }
    juce::String err;
    auto p = fmt.createInstanceFromDescription(*found[0], 48000, 512, err);
    if (!p) { printf("FAIL create: %s\n", err.toRawUTF8()); return 1; }
    std::unique_ptr<juce::AudioProcessorEditor> ed(p->createEditor());
    if (!ed) { printf("FAIL editor\n"); return 1; }
    if (argc > 3) { auto w = juce::String(argv[3]).getIntValue(); ed->setSize(w, ed->getHeight() * w / juce::jmax(1, ed->getWidth())); }
    ed->setOpaque(true);
    ed->addToDesktop(0);
    ed->setTopLeftPosition(0, 0);
    ed->setVisible(true);
    juce::MessageManager::getInstance()->runDispatchLoopUntil(1500);
    auto cmd = juce::String("import -window root -crop ") + juce::String(ed->getWidth()) + "x" + juce::String(ed->getHeight())
             + "+0+0 " + juce::String(argv[2]).quoted();
    int rc = std::system(cmd.toRawUTF8());
    printf("%s %dx%d -> %s\n", rc == 0 ? "OK" : "FAIL", ed->getWidth(), ed->getHeight(), argv[2]);
    ed = nullptr;
    p = nullptr;
    return rc;
}
