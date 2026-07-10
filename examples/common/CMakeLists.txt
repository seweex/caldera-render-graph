
find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(glm CONFIG REQUIRED)

add_library(caldera-examples-common STATIC
        src/window.cpp
        src/device.cpp
        src/swapchain.cpp
        src/frame.cpp
        src/descriptor.cpp
        src/memory.cpp
        src/image.cpp
        src/buffer.cpp
        src/shader.cpp
        src/scheduler.cpp
        src/pipeline.cpp
        src/renderer.cpp)

target_include_directories(caldera-examples-common PUBLIC include)
target_link_libraries(caldera-examples-common PUBLIC
        glfw
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
        spdlog::spdlog
        glm::glm
        stb-image
        caldera-render-graph)

target_compile_features(caldera-examples-common PUBLIC cxx_std_20)
target_compile_definitions(caldera-examples-common PUBLIC SPDLOG_NO_EXCEPTIONS)

target_precompile_headers(caldera-examples-common PUBLIC <caldera-examples-common/config.h> <vk_mem_alloc.h>)