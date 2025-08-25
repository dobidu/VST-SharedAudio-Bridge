cmake_minimum_required(VERSION 3.24)
project(WaveGenerator VERSION 0.0.1)

# add MusicAI headers
include_directories(
        ${SharedAudioPOC_SOURCE_DIR}/include/musicai
)

# Define MusicAI lib targets
if(WIN32)
    # musicai
    add_library(musicai SHARED IMPORTED)
    set_target_properties(musicai PROPERTIES
            IMPORTED_LOCATION "${SharedAudioPOC_SOURCE_DIR}/lib/musicai.dll"
            IMPORTED_IMPLIB "${SharedAudioPOC_SOURCE_DIR}/lib/musicai.lib"
    )

    # sndfile
    add_library(sndfile SHARED IMPORTED)
    set_target_properties(sndfile PROPERTIES
            IMPORTED_LOCATION "${SharedAudioPOC_SOURCE_DIR}/deps/sndfile.dll"
            IMPORTED_IMPLIB "${SharedAudioPOC_SOURCE_DIR}/deps/sndfile.lib"
    )

    # onnxruntime
    add_library(onnxruntime SHARED IMPORTED)
    set_target_properties(onnxruntime PROPERTIES
            IMPORTED_LOCATION "${SharedAudioPOC_SOURCE_DIR}/deps/onnxruntime.dll"
            IMPORTED_IMPLIB "${SharedAudioPOC_SOURCE_DIR}/deps/onnxruntime.lib"
    )
endif()

# Juce CMake definitions
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
        FileManagementPage.cpp
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
        musicai
        sndfile
        onnxruntime
    PUBLIC
        ZMQCommServer
        MemoryManager
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Copies DLLs to the .exe (bin) folder when building
if(WIN32)
    foreach(DLL_NAME musicai.dll sndfile.dll onnxruntime.dll)
        if(EXISTS "${SharedAudioPOC_SOURCE_DIR}/lib/${DLL_NAME}")
            add_custom_command(TARGET WaveGenerator POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${SharedAudioPOC_SOURCE_DIR}/lib/${DLL_NAME}"
                    $<TARGET_FILE_DIR:WaveGenerator>
            )
        elseif(EXISTS "${SharedAudioPOC_SOURCE_DIR}/deps/${DLL_NAME}")
            add_custom_command(TARGET WaveGenerator POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${SharedAudioPOC_SOURCE_DIR}/deps/${DLL_NAME}"
                    $<TARGET_FILE_DIR:WaveGenerator>
            )
        endif()
    endforeach()
endif()