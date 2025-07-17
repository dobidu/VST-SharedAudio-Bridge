cmake_minimum_required(VERSION 3.31)
project(SharedAudioPOC)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(FetchContent)
FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.7
)
FetchContent_MakeAvailable(JUCE)

add_subdirectory("sources/MemoryManager")
add_subdirectory("sources/ZMQCommClient")
add_subdirectory("sources/ZMQCommServer")
add_subdirectory("plugins/AudioReceiver")
add_subdirectory("plugins/WaveGenerator")
