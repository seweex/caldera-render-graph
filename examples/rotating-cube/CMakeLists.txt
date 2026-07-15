
add_executable(caldera-rotating-cube src/main.cpp)

target_include_directories(caldera-rotating-cube PRIVATE include)
target_link_libraries(caldera-rotating-cube PRIVATE caldera-examples-common)

set(CALDERA_ROTATING_CUBE_SHADER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(CALDERA_ROTATING_CUBE_SHADER_EXPORT "${CMAKE_CURRENT_SOURCE_DIR}/include/compiled-shaders")

declare_shader_compilation(
    ${CALDERA_ROTATING_CUBE_SHADER_SOURCE}
    ${CALDERA_ROTATING_CUBE_SHADER_EXPORT}
    caldera-rotating-cube)