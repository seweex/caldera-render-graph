cmake_minimum_required(VERSION 3.25)

# Configuration

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CALDERA_USE_SANITIZERS_DEFAULT_VAL ON)
else()
    set(CALDERA_USE_SANITIZERS_DEFAULT_VAL OFF)
endif()

option(CALDERA_BUILD_EXAMPLES "Build example executable applications" OFF)
option(CALDERA_USE_SANITIZERS "Enable sanitizers (address & undefined behavior)" CALDERA_USE_SANITIZERS_DEFAULT_VAL)
option(CALDERA_GEN_DOCS "Enables the docs generation target" OFF)

function(run_conan_install CONAN_FILE_NAME)
    message(STATUS "[CONAN] Install dependencies from ${CONAN_FILE_NAME}")

    execute_process(COMMAND conan install ${CMAKE_SOURCE_DIR}/${CONAN_FILE_NAME}
        --build=missing
        -s build_type=${CMAKE_BUILD_TYPE}
        -c tools.cmake.cmaketoolchain:user_presets=False
        -verror
        COMMAND_ERROR_IS_FATAL ANY)
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/conan_install")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} PARENT_SCOPE)

    message(STATUS "[CONAN] Install finished successfully")
endfunction()

if(CALDERA_BUILD_EXAMPLES)
    run_conan_install("conanfile_examples.txt")
else()
    run_conan_install("conanfile_graph.txt")
endif()

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/generators/conan_toolchain.cmake" CACHE STRING "" FORCE)

# Project

project(caldera
        DESCRIPTION "A lightweight and easily extensible library for automatic management of Vulkan barriers and resources"
        VERSION 0.1.0
        LANGUAGES CXX)

find_package(Git REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

add_subdirectory(render-graph)

if(CALDERA_BUILD_EXAMPLES)
    add_subdirectory(external/stb)
    include(examples/common/ShaderCompile.cmake)

    add_subdirectory(examples/common)
    add_subdirectory(examples/rotating-cube)
    add_subdirectory(examples/simulation)
endif()

if(CALDERA_GEN_DOCS)
    add_subdirectory(docs)
endif()