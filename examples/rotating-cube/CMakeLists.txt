
add_executable(caldera-rotating-cube src/main.cpp)

target_include_directories(caldera-rotating-cube PRIVATE include)
target_link_libraries(caldera-rotating-cube PRIVATE caldera-examples-common)