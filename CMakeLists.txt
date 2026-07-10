cmake_minimum_required(VERSION 3.25)

project(caldera
        DESCRIPTION "A lightweight and easily extensible library for automatic management of Vulkan barriers and resources"
        VERSION 0.1.0
        LANGUAGES CXX)

find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

add_subdirectory(external/stb)
add_subdirectory(render-graph)

add_subdirectory(examples/common)
add_subdirectory(examples/rotating-cube)