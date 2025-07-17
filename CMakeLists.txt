cmake_minimum_required(VERSION 3.10)
project(cli-musicai VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 17)

# Output dirs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# add headers
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# source code
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Define targets 
if(WIN32)
    # musicai
    add_library(musicai SHARED IMPORTED)
    set_target_properties(musicai PROPERTIES
        IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/lib/musicai.dll"
        IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/lib/musicai.lib"
    )

    # sndfile
    add_library(sndfile SHARED IMPORTED)
    set_target_properties(sndfile PROPERTIES
        IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/deps/sndfile.dll"
        IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/deps/sndfile.lib"
    )

    # onnxruntime
    add_library(onnxruntime SHARED IMPORTED)
    set_target_properties(onnxruntime PROPERTIES
        IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/deps/onnxruntime.dll"
        IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/deps/onnxruntime.lib"
    )
endif()

add_executable(cli-musicai ${SOURCES})

# Link IMPORTED TARGETS
if(WIN32)
    target_link_libraries(cli-musicai PRIVATE musicai sndfile onnxruntime)
else()
    # If it is Linux/Mac, adapt it here for other linkages...
endif()

# Copies DLLs to the .exe (bin) folder when building
if(WIN32)
    foreach(DLL_NAME musicai.dll sndfile.dll onnxruntime.dll)
        if(EXISTS "${PROJECT_SOURCE_DIR}/lib/${DLL_NAME}")
            add_custom_command(TARGET cli-musicai POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${PROJECT_SOURCE_DIR}/lib/${DLL_NAME}"
                $<TARGET_FILE_DIR:cli-musicai>
            )
        elseif(EXISTS "${PROJECT_SOURCE_DIR}/deps/${DLL_NAME}")
            add_custom_command(TARGET cli-musicai POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${PROJECT_SOURCE_DIR}/deps/${DLL_NAME}"
                $<TARGET_FILE_DIR:cli-musicai>
            )
        endif()
    endforeach()
endif()
