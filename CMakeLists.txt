cmake_minimum_required(VERSION 3.25)

project(caldera
        DESCRIPTION "A lightweight and easily extensible library for automatic management of Vulkan barriers and resources"
        VERSION 0.0.1
        LANGUAGES CXX)

find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

add_subdirectory(render-graph)
add_subdirectory(example)