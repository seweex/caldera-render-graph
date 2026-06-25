
find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)

add_executable(caldera-example
        src/main.cpp
        src/window.cpp)

target_include_directories(caldera-example PRIVATE include)
target_link_libraries(caldera-example PRIVATE
        glfw
        Vulkan::Vulkan
        spdlog::spdlog)

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