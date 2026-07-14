
add_executable(caldera-simulation
        src/main.cpp
        src/graphics.cpp
        src/pipeline.cpp
        src/descriptors.cpp)

target_include_directories(caldera-simulation PRIVATE include)
target_link_libraries(caldera-simulation PRIVATE caldera-examples-common)

set(CALDERA_SIMULATION_SHADER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(CALDERA_SIMULATION_SHADER_EXPORT "${CMAKE_CURRENT_SOURCE_DIR}/include/compiled-shaders")

declare_shader_compilation(
    ${CALDERA_SIMULATION_SHADER_SOURCE}
    ${CALDERA_SIMULATION_SHADER_EXPORT}
    caldera-simulation)