cmake_minimum_required(VERSION 3.22)
project(SoundWave VERSION 0.1.0)
add_subdirectory(JUCE)

# Open-licensed fonts (OFL, see fonts/*-OFL.txt) used by plugins/SWDesign.h
juce_add_binary_data(SWFonts SOURCES fonts/Doto.ttf fonts/SpaceMono-Regular.ttf fonts/SpaceMono-Bold.ttf)

function(sw_plugin NAME CODE)  # extra args (e.g. IS_SYNTH TRUE) go to juce_add_plugin
  juce_add_plugin(${NAME} ${ARGN}
    COMPANY_NAME "SoundWave"
    PLUGIN_MANUFACTURER_CODE Sndw
    PLUGIN_CODE ${CODE}
    PRODUCT_NAME "SW ${NAME}"
    FORMATS VST3
    COPY_PLUGIN_AFTER_BUILD FALSE)
  target_sources(${NAME} PRIVATE plugins/${NAME}.cpp)
  target_include_directories(${NAME} PRIVATE plugins)
  target_compile_definitions(${NAME} PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
  target_link_libraries(${NAME} PRIVATE SWFonts juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
endfunction()

sw_plugin(Prism   Prsm)
sw_plugin(Squeeze Sqze)
sw_plugin(Glow    Glow)
sw_plugin(Flux    Flux)
sw_plugin(Wall    Wall)
sw_plugin(Gate    Gate)
sw_plugin(Space   Spce)
sw_plugin(Echo    Echo)
sw_plugin(Swirl   Swrl)
sw_plugin(Jet     Jetf)
sw_plugin(Orbit   Orbt)
sw_plugin(Sway    Sway)
sw_plugin(Room    Room)
sw_plugin(Crush   Crsh)
sw_plugin(Tune    Tune)
sw_plugin(Tide    Tide IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE)

juce_add_console_app(sw_check PRODUCT_NAME "sw_check")
target_sources(sw_check PRIVATE tools/check.cpp)
target_compile_definitions(sw_check PRIVATE JUCE_PLUGINHOST_VST3=1 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(sw_check PRIVATE juce::juce_audio_processors juce::juce_recommended_config_flags)

# Screenshot helper: opens a VST3 editor on the X display (use with xvfb-run + ImageMagick import)
juce_add_console_app(sw_shot PRODUCT_NAME "sw_shot")
target_sources(sw_shot PRIVATE tools/shot.cpp)
target_compile_definitions(sw_shot PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1 JUCE_PLUGINHOST_VST3=1 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(sw_shot PRIVATE X11 juce::juce_audio_processors juce::juce_recommended_config_flags)
