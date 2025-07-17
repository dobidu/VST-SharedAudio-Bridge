set(HEADER_LIST "${SharedAudioPOC_SOURCE_DIR}/include/MemoryManager.h")
add_library(MemoryManager MemoryManager.cpp ${HEADER_LIST})
target_include_directories(MemoryManager PUBLIC ${SharedAudioPOC_SOURCE_DIR}/include)
source_group(
        TREE "${PROJECT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${HEADER_LIST}
)
#target_link_libraries(MemoryManager
#        PRIVATE
#        juce::juce_audio_basics
#)