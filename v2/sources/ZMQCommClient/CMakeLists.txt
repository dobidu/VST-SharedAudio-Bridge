set(HEADER_LIST "${SharedAudioPOC_SOURCE_DIR}/include/ZMQCommClient.h")
find_package(cppzmq)
add_library(ZMQCommClient ZMQCommClient.cpp ${HEADER_LIST})
target_include_directories(ZMQCommClient PUBLIC ${SharedAudioPOC_SOURCE_DIR}/include)
source_group(
        TREE "${PROJECT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${HEADER_LIST}
)
target_link_libraries(ZMQCommClient cppzmq-static)