cmake_minimum_required(VERSION 3.15)

project(LowLatencyAudioPlugin VERSION 1.0.0)

# Definir padrão C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Opções específicas do sistema operacional
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Definir o caminho para o JUCE
set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE" CACHE PATH "Caminho para o JUCE")

# Verificar se JUCE existe
if(NOT EXISTS ${JUCE_PATH})
    message(FATAL_ERROR "JUCE não encontrado em ${JUCE_PATH}")
endif()

# Adicionar o JUCE como submódulo
add_subdirectory(${JUCE_PATH} JUCE)

# Incluir as ferramentas extras do JUCE
include(${JUCE_PATH}/extras/Build/CMake/JUCEUtils.cmake)

# Configurações do plugin
juce_add_plugin(LowLatencyAudioPlugin
    VERSION                     1.0.0
    COMPANY_NAME                "YourCompany"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD     TRUE
    PLUGIN_MANUFACTURER_CODE    Ymnf
    PLUGIN_CODE                 LwLt
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "Low Latency Audio Plugin"
)

juce_generate_juce_header(LowLatencyAudioPlugin)


# Arquivos fonte
target_sources(LowLatencyAudioPlugin
    PRIVATE
        LowLatencyAudioPlugin.cpp
        LowLatencyAudioProcessorEditor.cpp
        SharedMemoryManager.cpp
)

# Módulos JUCE necessários
target_compile_definitions(LowLatencyAudioPlugin
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
)

# Incluir diretórios
target_include_directories(LowLatencyAudioPlugin 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${JUCE_PATH}/modules
        ${CMAKE_CURRENT_BINARY_DIR}/JuceLibraryCode
)

# Vincular módulos JUCE explicitamente
target_link_libraries(LowLatencyAudioPlugin
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_extra
        juce::juce_gui_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_basics
        juce::juce_data_structures
        juce::juce_events
        juce::juce_core
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Configurações específicas de plataforma
if(WIN32)
    target_compile_definitions(LowLatencyAudioPlugin PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(LowLatencyAudioPlugin PRIVATE wsock32 ws2_32)
endif()

if(LINUX)
    target_link_libraries(LowLatencyAudioPlugin PRIVATE pthread rt)
endif()

if(APPLE)
    target_compile_options(LowLatencyAudioPlugin PRIVATE -Wno-deprecated-declarations)
endif()