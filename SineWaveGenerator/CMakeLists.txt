cmake_minimum_required(VERSION 3.15)

project(SineWaveGenerator VERSION 1.0.0)

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

# Arquivos fonte - especificar caminhos completos 
set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/SineWaveGenerator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SharedMemoryManager.cpp"
)

# Usar add_executable padrão 
add_executable(SineWaveGenerator ${SOURCES})

# Incluir diretórios
target_include_directories(SineWaveGenerator 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${JUCE_PATH}/modules
)

# Configurar definições
target_compile_definitions(SineWaveGenerator
    PRIVATE
    JUCE_STANDALONE_APPLICATION=1
    JUCE_REPORT_APP_USAGE=0
    JUCE_WEB_BROWSER=0 
    JUCE_USE_CURL=0 
    JUCE_APPLICATION_ENTRY_POINT=0  # Crucial: isso evita que o JUCE redefina o ponto de entrada
)

# Vincular bibliotecas JUCE
target_link_libraries(SineWaveGenerator
    PRIVATE
    juce::juce_core
    juce::juce_events
    juce::juce_data_structures
    juce::juce_graphics
    juce::juce_gui_basics
)

# Configurações específicas de plataforma
if(WIN32)
    target_compile_definitions(SineWaveGenerator PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(SineWaveGenerator PRIVATE wsock32 ws2_32)
endif()

if(LINUX)
    target_link_libraries(SineWaveGenerator PRIVATE pthread rt)
endif()

if(APPLE)
    target_compile_options(SineWaveGenerator PRIVATE -Wno-deprecated-declarations)
    target_link_libraries(SineWaveGenerator PRIVATE "-framework CoreFoundation")
endif()

# Mensagens de debug
message(STATUS "Arquivos fonte: ${SOURCES}")
message(STATUS "Diretório atual: ${CMAKE_CURRENT_SOURCE_DIR}")