cmake_minimum_required(VERSION 3.24)

# Change to your project name
project(WaveGenerator VERSION 0.0.1)

juce_add_gui_app(WaveGenerator
        VERSION 0.0.1
        PRODUCT_NAME "WaveGenerator"
)

juce_generate_juce_header(WaveGenerator)

target_sources(WaveGenerator
        PRIVATE
        Main.cpp
        MainComponent.cpp
        AudioServer.cpp
)

target_compile_definitions(WaveGenerator
        PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="<TARGET_PROPERTY:WaveGenerator, JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="<TARGET_PROPERTY:WaveGenerator, JUCE_VERSION>"
)

target_link_libraries(WaveGenerator
    PRIVATE
        juce::juce_core
        juce::juce_gui_extra
        juce::juce_audio_basics
        juce::juce_audio_formats
    PUBLIC
        ZMQCommServer
        MemoryManager
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)