
add_library(caldera-render-graph STATIC
        src/pass.cpp
        src/graph.cpp
        src/scheduler.cpp)

target_include_directories(caldera-render-graph PUBLIC include)
target_link_libraries(caldera-render-graph PRIVATE
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator)

target_compile_features(caldera-render-graph PUBLIC cxx_std_20)

target_compile_definitions(caldera-render-graph PUBLIC
        VULKAN_HPP_NO_EXCEPTIONS
        GLFW_INCLUDE_VULKAN)

target_precompile_headers(caldera-render-graph PUBLIC <caldera-render-graph/config.h>)

if(MSVC)
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    target_compile_options(caldera-render-graph PUBLIC /EHs-c-)
else()
    target_compile_options(caldera-render-graph PUBLIC -fno-exceptions)
endif()

if(CALDERA_USE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(caldera-render-graph PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(caldera-render-graph PUBLIC -fsanitize=address,undefined)
    elseif(MSVC)
        target_compile_options(caldera-render-graph PUBLIC $<$<CONFIG:Debug>:/fsanitize=address>)
        target_link_options(caldera-render-graph PUBLIC /fsanitize=address)
    endif()
endif()