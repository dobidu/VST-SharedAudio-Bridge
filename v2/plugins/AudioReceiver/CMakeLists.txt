cmake_minimum_required(VERSION 3.24)

# Change to your project name
project(AudioReceiver VERSION 0.0.1)

# Change these to your own preferences
juce_add_plugin(AudioReceiver
        PLUGIN_MANUFACTURER_CODE Mois
        PLUGIN_CODE Dgin
        FORMATS VST3 Standalone
        PRODUCT_NAME "AudioReceiver"
)

# Make the SourceFiles buildable
target_sources(AudioReceiver
    PRIVATE
        PluginEditor.cpp
        PluginProcessor.cpp
)

# These are some toggleable options from the JUCE CMake API
target_compile_definitions(AudioReceiver
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# JUCE libraries to bring into our project
target_link_libraries(AudioReceiver
        PRIVATE
        juce::juce_analytics
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_core
        juce::juce_data_structures
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_utils
        juce::juce_dsp
        PUBLIC
        ZMQCommClient
        MemoryManager
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)