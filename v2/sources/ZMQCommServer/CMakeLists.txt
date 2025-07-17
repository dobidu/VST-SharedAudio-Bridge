set(HEADER_LIST "${SharedAudioPOC_SOURCE_DIR}/include/ZMQCommServer.h")
find_package(cppzmq)
add_library(ZMQCommServer ZMQCommServer.cpp ${HEADER_LIST})
target_include_directories(ZMQCommServer PUBLIC ${SharedAudioPOC_SOURCE_DIR}/include)
source_group(
        TREE "${PROJECT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${HEADER_LIST}
)
target_link_libraries(ZMQCommServer cppzmq-static)