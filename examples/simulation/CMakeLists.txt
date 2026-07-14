
add_executable(caldera-simulation
        src/main.cpp
        src/graphics.cpp
        src/pipeline.cpp
        src/descriptors.cpp)

target_include_directories(caldera-simulation PRIVATE include)
target_link_libraries(caldera-simulation PRIVATE caldera-examples-common)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(CALDERA_SIMULATION_SHADER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(CALDERA_SIMULATION_SHADER_OUTPUT "${CMAKE_BINARY_DIR}/compiled-shaders")
set(CALDERA_SIMULATION_SHADER_EXPORT "${CMAKE_CURRENT_SOURCE_DIR}/include/compiled-shaders")

set(SHADER_LINK_ROOT "${CMAKE_SOURCE_DIR}/external/shader-link")

file(MAKE_DIRECTORY "${CALDERA_SIMULATION_SHADER_OUTPUT}")
file(MAKE_DIRECTORY "${CALDERA_SIMULATION_SHADER_EXPORT}")

add_custom_target(shader-link-compilation ALL
        COMMAND ${Python3_EXECUTABLE} -m shader_link
        --input  "${CALDERA_SIMULATION_SHADER_SOURCE}"
        --output "${CALDERA_SIMULATION_SHADER_OUTPUT}"
        --export "${CALDERA_SIMULATION_SHADER_EXPORT}"
        --args   "-O --target-env=vulkan1.4"
        -f
        WORKING_DIRECTORY "${SHADER_LINK_ROOT}"
        DEPENDS "${SHADER_LINK_ROOT}")

add_dependencies(caldera-simulation shader-link-compilation)