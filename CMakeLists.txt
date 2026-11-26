cmake_minimum_required(VERSION 3.22)
project(YoloMonkMachine VERSION 2.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")   # universal binary
endif()

# Which formats to build. Override e.g. -DMONK_FORMATS="VST3"
if(APPLE)
    set(MONK_FORMATS "VST3;AU;Standalone" CACHE STRING "Plugin formats")
else()
    set(MONK_FORMATS "VST3;Standalone" CACHE STRING "Plugin formats")
endif()

# Use a local JUCE checkout with -DJUCE_DIR=/path/to/JUCE, otherwise download it
set(JUCE_DIR "" CACHE PATH "Path to an existing JUCE checkout (optional)")
if(JUCE_DIR)
    add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
        GIT_SHALLOW    ON)
    FetchContent_MakeAvailable(JUCE)
endif()

if(APPLE)
    set(MONK_COPY_AFTER_BUILD TRUE)    # installs into ~/Library/Audio/Plug-Ins
else()
    set(MONK_COPY_AFTER_BUILD FALSE)   # Windows/Linux: copy the .vst3 yourself (see README)
endif()

juce_add_plugin(YoloMonkMachine
    COMPANY_NAME             "YOLO"
    PRODUCT_NAME             "YOLO Monk Machine"
    BUNDLE_ID                com.yolo.monkmachine
    IS_SYNTH                 TRUE
    NEEDS_MIDI_INPUT         TRUE
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD  ${MONK_COPY_AFTER_BUILD}
    PLUGIN_MANUFACTURER_CODE Yolo
    PLUGIN_CODE              YMnk
    VST3_CATEGORIES          Instrument Synth
    FORMATS                  ${MONK_FORMATS})

target_sources(YoloMonkMachine PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_include_directories(YoloMonkMachine PRIVATE Source)

target_compile_definitions(YoloMonkMachine PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

# Headless Linux builds without ALSA/JACK dev packages (plugin formats don't need them)
option(MONK_NO_LINUX_AUDIO_DEVICES "Build without ALSA/JACK (Linux)" OFF)
if(MONK_NO_LINUX_AUDIO_DEVICES)
    target_compile_definitions(YoloMonkMachine PUBLIC JUCE_ALSA=0 JUCE_JACK=0)
endif()

target_link_libraries(YoloMonkMachine
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Optional: offline DSP test that renders demo.wav (no JUCE needed)
add_executable(monk_render_test EXCLUDE_FROM_ALL tests/render_test.cpp)
target_include_directories(monk_render_test PRIVATE Source)

# Dev tool: screenshots of the editor + processor smoke test (not built by default)
option(MONK_BUILD_SNAPSHOT "Build the editor snapshot tool" OFF)
if(MONK_BUILD_SNAPSHOT)
    juce_add_console_app(monk_snapshot PRODUCT_NAME "monk_snapshot")
    target_sources(monk_snapshot PRIVATE tests/snapshot.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_include_directories(monk_snapshot PRIVATE Source)
    target_compile_definitions(monk_snapshot PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JucePlugin_Name="YOLO Monk Machine"
        JUCE_STANDALONE_APPLICATION=1 JUCE_ALSA=0 JUCE_JACK=0)
    target_link_libraries(monk_snapshot PRIVATE juce::juce_audio_utils juce::juce_recommended_config_flags)
endif()
