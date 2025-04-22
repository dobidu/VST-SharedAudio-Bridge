cmake_minimum_required(VERSION 3.15)

project(SineWaveGenerator VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE" CACHE PATH "JUCE Path")

if(NOT EXISTS ${JUCE_PATH})
    message(FATAL_ERROR "JUCE not found in ${JUCE_PATH}")
endif()

add_subdirectory(${JUCE_PATH} JUCE)

set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/SineWaveGenerator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SharedMemoryManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AudioFileReader.cpp"
)

add_executable(SineWaveGenerator ${SOURCES})

target_include_directories(SineWaveGenerator 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${JUCE_PATH}/modules
)

target_compile_definitions(SineWaveGenerator
    PRIVATE
    JUCE_STANDALONE_APPLICATION=1
    JUCE_REPORT_APP_USAGE=0
    JUCE_WEB_BROWSER=0 
    JUCE_USE_CURL=0 
    JUCE_APPLICATION_ENTRY_POINT=0  
)

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

message(STATUS "Source files: ${SOURCES}")
message(STATUS "Current dir: ${CMAKE_CURRENT_SOURCE_DIR}")