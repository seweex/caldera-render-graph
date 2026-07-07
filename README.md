# Caldera – Render Graph

A lightweight and easily extensible library for automatic management of Vulkan barriers and resources

### Features

- Automatic sync barriers generation
- Simple architecture
- Exception-free

### Concept

1. **Setup Phase**: adding passes and resources
2. **Compile Phase**: making optimal graph of passes and resource usage
3. **Execute Phase**: traversing the compiled graph and writing Vulkan command

### Requirements

- C++20 compiler
- CMake (3.25+)
- Vulkan SDK (1.4+)
- Conan (2.0+)

### Dependencies

- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)

for examples only:

- [GLFW](https://github.com/glfw/glfw)
- [spdlog](https://github.com/gabime/spdlog)
- [glm](https://github.com/g-truc/glm)

## Building

### 0. Install Vulkan SDK

Go to [official Vulkan SDK source](https://vulkan.lunarg.com/sdk/home) and install it

### 1. Clone the repository:

```bash
git clone https://github.com/seweex/caldera-render-graph.git
cd caldera-render-graph
```

### 2. Via `conan` package manager

```bash
conan install . --output-folder=build --build=missing
cmake --preset conan-release
cmake --build --preset conan-release
```

or for debug preset

```bash
conan install . -s build_type=Debug --output-folder=build --build=missing
cmake --preset conan-debug
cmake --build --preset conan-debug
```

### 2b. Via manual package installation

- Install all [dependencies](#dependencies)
- Make sure it's available to be found by _find_package()_
- Then call this (specify your generator):
  ```bash
  mkdir build
  cd build
  cmake -G "Ninja" ..
  cmake --build .  
  ```

