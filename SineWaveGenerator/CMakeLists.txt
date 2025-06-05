cmake_minimum_required(VERSION 3.15)
project(SineWaveGenerator VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Configuração do JUCE
set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../submodules/JUCE" CACHE PATH "JUCE Path")
if(NOT EXISTS ${JUCE_PATH})
    message(FATAL_ERROR "JUCE not found in ${JUCE_PATH}")
endif()
add_subdirectory(${JUCE_PATH} JUCE)

# Configuração do MusicAI
set(MUSICAI_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../submodules/libmusicai-ableton" CACHE PATH "MUSICAI LIB PATH")
if(NOT EXISTS ${MUSICAI_LIB_PATH})
    message(FATAL_ERROR "MUSICAI LIB not found in ${MUSICAI_LIB_PATH}")
endif()

# Desativar a compilação de exemplos do MusicAI
set(MUSICAI_BUILD_EXAMPLES OFF CACHE BOOL "Build MusicAI examples" FORCE)

# Adicionar o submódulo MusicAI
add_subdirectory(${MUSICAI_LIB_PATH} MUSICAI)

# Verificar caminhos das bibliotecas (diagnóstico)
message(STATUS "MUSICAI_LIB_PATH: ${MUSICAI_LIB_PATH}")
message(STATUS "MUSICAI_LIB_DIR: ${MUSICAI_LIB_DIR}")
message(STATUS "MUSICAI_DEPS_DIR: ${MUSICAI_DEPS_DIR}")
message(STATUS "MUSICAI_INCLUDE_DIRS: ${MUSICAI_INCLUDE_DIRS}")
message(STATUS "MUSICAI_LIBRARIES: ${MUSICAI_LIBRARIES}")

# Verificar se os arquivos existem
if(WIN32)
    if(NOT EXISTS "${MUSICAI_LIB_PATH}/lib/musicai.lib")
        message(FATAL_ERROR "musicai.lib not found at ${MUSICAI_LIB_PATH}/lib/musicai.lib")
    endif()
    if(NOT EXISTS "${MUSICAI_LIB_PATH}/deps/sndfile.lib")
        message(FATAL_ERROR "sndfile.lib not found at ${MUSICAI_LIB_PATH}/deps/sndfile.lib")
    endif()
    if(NOT EXISTS "${MUSICAI_LIB_PATH}/deps/onnxruntime.lib")
        message(FATAL_ERROR "onnxruntime.lib not found at ${MUSICAI_LIB_PATH}/deps/onnxruntime.lib")
    endif()
endif()

# Fontes do SineWaveGenerator
set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/SineWaveGenerator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SharedMemoryManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AudioFileReader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MusicAIProcessor.cpp" # Novo arquivo
)

# Primeiro, crie o executável
add_executable(SineWaveGenerator ${SOURCES})

# Depois, configure os diretórios de inclusão
target_include_directories(SineWaveGenerator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${JUCE_PATH}/modules
        ${MUSICAI_INCLUDE_DIRS}
)

# Configure as definições de compilação
target_compile_definitions(SineWaveGenerator
    PRIVATE
    JUCE_STANDALONE_APPLICATION=1
    JUCE_REPORT_APP_USAGE=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_ENTRY_POINT=0  
)

# Configure as bibliotecas JUCE
target_link_libraries(SineWaveGenerator
    PRIVATE
    juce::juce_core
    juce::juce_events
    juce::juce_data_structures
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_dsp
)

# Adicionar bibliotecas MusicAI diretamente com caminhos completos
if(WIN32)
    target_link_libraries(SineWaveGenerator
        PRIVATE
        "${MUSICAI_LIB_PATH}/lib/musicai.lib"
        "${MUSICAI_LIB_PATH}/deps/sndfile.lib"
        "${MUSICAI_LIB_PATH}/deps/onnxruntime.lib"
        wsock32 ws2_32
    )
    
    # Copiar DLLs necessárias
    add_custom_command(TARGET SineWaveGenerator POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MUSICAI_LIB_PATH}/lib/musicai.dll"
            "${MUSICAI_LIB_PATH}/deps/onnxruntime.dll"
            "${MUSICAI_LIB_PATH}/deps/DirectML.dll"
            "${MUSICAI_LIB_PATH}/deps/sndfile.dll"
            $<TARGET_FILE_DIR:SineWaveGenerator>)
else()
    # Para Linux/Mac, use as variáveis exportadas
    target_link_libraries(SineWaveGenerator PRIVATE ${MUSICAI_LIBRARIES})
endif()

# Copiar modelos
add_custom_command(TARGET SineWaveGenerator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:SineWaveGenerator>/models
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MUSICAI_LIB_PATH}/models/vocals_others.mai"
        $<TARGET_FILE_DIR:SineWaveGenerator>/models/)

if(LINUX)
    target_link_libraries(SineWaveGenerator PRIVATE pthread rt)
endif()

if(APPLE)
    target_compile_options(SineWaveGenerator PRIVATE -Wno-deprecated-declarations)
    target_link_libraries(SineWaveGenerator PRIVATE "-framework CoreFoundation")
endif()

message(STATUS "Source files: ${SOURCES}")
message(STATUS "Current dir: ${CMAKE_CURRENT_SOURCE_DIR}")