cmake_minimum_required(VERSION 3.10)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)

# Project name and version
project(MusicAI VERSION ${PROJECT_VERSION})

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Option to control building the example app
option(MUSICAI_BUILD_EXAMPLES "Build MusicAI example applications" ON)

# Define paths
set(DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Include directories
include_directories(${INCLUDE_DIR})

# Create imported targets for the libraries so they can be used by other projects
if(WIN32)
    # Windows libraries
    add_library(musicai SHARED IMPORTED)
    set_target_properties(musicai PROPERTIES
        IMPORTED_LOCATION "${LIB_DIR}/musicai.dll"
        IMPORTED_IMPLIB "${LIB_DIR}/musicai.lib"
    )

    add_library(sndfile SHARED IMPORTED)
    set_target_properties(sndfile PROPERTIES
        IMPORTED_LOCATION "${DEPS_DIR}/sndfile.dll"
        IMPORTED_IMPLIB "${DEPS_DIR}/sndfile.lib"
    )

    add_library(onnxruntime SHARED IMPORTED)
    set_target_properties(onnxruntime PROPERTIES
        IMPORTED_LOCATION "${DEPS_DIR}/onnxruntime.dll"
        IMPORTED_IMPLIB "${DEPS_DIR}/onnxruntime.lib"
    )
else()
    # Linux/macOS libraries
    find_library(MUSICAI_LIB_PATH musicai PATHS ${LIB_DIR} NO_DEFAULT_PATH)
    add_library(musicai SHARED IMPORTED)
    set_target_properties(musicai PROPERTIES
        IMPORTED_LOCATION "${MUSICAI_LIB_PATH}"
    )

    find_library(ONNX_LIB_PATH onnxruntime PATHS ${DEPS_DIR} NO_DEFAULT_PATH)
    add_library(onnxruntime SHARED IMPORTED)
    set_target_properties(onnxruntime PROPERTIES
        IMPORTED_LOCATION "${ONNX_LIB_PATH}"
    )
endif()

# Example executable (only build if MUSICAI_BUILD_EXAMPLES is ON)
if(MUSICAI_BUILD_EXAMPLES)
    add_executable(example_app ${CMAKE_CURRENT_SOURCE_DIR}/examples/example.c)
    
    # Link with the imported libraries
    target_link_libraries(example_app PRIVATE musicai sndfile onnxruntime)
    
    # Copy DLLs to output directory for Windows
    if(WIN32)
        add_custom_command(TARGET example_app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LIB_DIR}/musicai.dll"
                "${DEPS_DIR}/onnxruntime.dll"
                "${DEPS_DIR}/DirectML.dll"
                "${DEPS_DIR}/sndfile.dll"
                $<TARGET_FILE_DIR:example_app>)
    endif()
    
    # Copy models to output directory
    add_custom_command(TARGET example_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:example_app>/models
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/models/vocals_others.mai"
            $<TARGET_FILE_DIR:example_app>/models/)
    
    # Install rules for example
    install(TARGETS example_app
            RUNTIME DESTINATION bin)
endif()

# Install models
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/models/
        DESTINATION bin/models)

# Install libraries
if(WIN32)
    install(FILES
            "${LIB_DIR}/musicai.dll"
            "${DEPS_DIR}/onnxruntime.dll"
            "${DEPS_DIR}/DirectML.dll"
            "${DEPS_DIR}/sndfile.dll"
            DESTINATION bin)
endif()

# Export MusicAI include directories and libraries for parent projects
set(MUSICAI_INCLUDE_DIRS ${INCLUDE_DIR} CACHE INTERNAL "MusicAI include directories")
set(MUSICAI_LIBRARIES musicai sndfile onnxruntime CACHE INTERNAL "MusicAI libraries")
set(MUSICAI_DEPS_DIR ${DEPS_DIR} CACHE INTERNAL "MusicAI dependencies directory")
set(MUSICAI_LIB_DIR ${LIB_DIR} CACHE INTERNAL "MusicAI library directory")
set(MUSICAI_MODELS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/models CACHE INTERNAL "MusicAI models directory")

# Function to help parent projects copy necessary DLLs and models
function(musicai_copy_dependencies TARGET_NAME)
    if(WIN32)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LIB_DIR}/musicai.dll"
                "${DEPS_DIR}/onnxruntime.dll"
                "${DEPS_DIR}/DirectML.dll"
                "${DEPS_DIR}/sndfile.dll"
                $<TARGET_FILE_DIR:${TARGET_NAME}>)
    endif()
    
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${TARGET_NAME}>/models
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/models/vocals_others.mai"
            $<TARGET_FILE_DIR:${TARGET_NAME}>/models/)
endfunction()