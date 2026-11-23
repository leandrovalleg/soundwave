cmake_minimum_required(VERSION 3.22)
project(SoundWaveApp VERSION 0.1.0)

set(TE ${CMAKE_CURRENT_SOURCE_DIR}/../tracktion_engine)
add_subdirectory(${TE}/modules/juce ${CMAKE_BINARY_DIR}/juce)
add_subdirectory(${TE}/modules ${CMAKE_BINARY_DIR}/tracktion)

juce_add_gui_app(SoundWave
    PRODUCT_NAME "SoundWave"
    COMPANY_NAME "SoundWave"
    NEEDS_CURL FALSE
    NEEDS_WEB_BROWSER FALSE
    MICROPHONE_PERMISSION_ENABLED TRUE
    ICON_BIG ../brand/icon.png)

juce_add_binary_data(SWAppFonts SOURCES
    ../fonts/Doto.ttf ../fonts/SpaceMono-Regular.ttf ../fonts/SpaceMono-Bold.ttf)

target_sources(SoundWave PRIVATE Main.cpp)

# MP3 encoder (LAME, LGPL) embedded in the exe so the app is a single file. CI passes -DSW_LAME_LIB=<path>.
if (SW_LAME_LIB AND EXISTS "${SW_LAME_LIB}")
    juce_add_binary_data(SWLame HEADER_NAME LameData.h NAMESPACE LameData SOURCES "${SW_LAME_LIB}")
    target_link_libraries(SoundWave PRIVATE SWLame)
    target_compile_definitions(SoundWave PRIVATE SW_EMBEDDED_LAME=1)
endif()
target_compile_features(SoundWave PRIVATE cxx_std_20)
target_include_directories(SoundWave PRIVATE ../plugins)
target_compile_definitions(SoundWave PRIVATE
    JUCE_PLUGINHOST_VST3=1
    $<$<PLATFORM_ID:Darwin>:JUCE_PLUGINHOST_AU=1>
    $<$<BOOL:$ENV{VST2_SDK_DIR}>:JUCE_PLUGINHOST_VST=1>
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_USE_CURL=0 JUCE_WEB_BROWSER=0
    TRACKTION_ENABLE_TIMESTRETCH_SOUNDTOUCH=1
    TRACKTION_ENABLE_LIBLAME=1)
if (DEFINED ENV{VST2_SDK_DIR})
    juce_set_vst2_sdk_path($ENV{VST2_SDK_DIR})
endif()
target_link_libraries(SoundWave PRIVATE
    tracktion::tracktion_core tracktion::tracktion_engine tracktion::tracktion_graph
    juce::juce_audio_devices juce::juce_audio_processors juce::juce_audio_utils
    SWAppFonts)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(SoundWave PRIVATE "-latomic")
endif()
if (MSVC)
    set_property(TARGET SoundWave PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
