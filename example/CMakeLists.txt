
find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)

add_executable(caldera-example
        src/main.cpp
        src/window.cpp
        src/device.cpp
        src/swapchain.cpp
        src/frame.cpp
        src/shader.cpp
        src/descriptor.cpp
        src/memory.cpp)

target_include_directories(caldera-example PRIVATE include)
target_link_libraries(caldera-example PRIVATE
        glfw
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
        spdlog::spdlog)

target_compile_features(caldera-example PRIVATE cxx_std_20)

target_compile_definitions(caldera-example PRIVATE
        VULKAN_HPP_NO_EXCEPTIONS
        SPDLOG_NO_EXCEPTIONS
        GLFW_INCLUDE_VULKAN)

if(MSVC)
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    target_compile_options(caldera-example PRIVATE /EHs-c-)
else()
    target_compile_options(caldera-example PRIVATE -fno-exceptions)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(caldera-example PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(caldera-example PRIVATE -fsanitize=address,undefined)
    elseif(MSVC)
        target_compile_options(my_vulkan_app PRIVATE $<$<CONFIG:Debug>:/fsanitize=address>)
        target_link_options(caldera-example PRIVATE /fsanitize=address)
    endif()
endif()