# Caldera – Render Graph

A lightweight and easily extensible library for automatic management of Vulkan barriers and resources

<div style="display: flex; align-items: stretch; gap: 10px;">
  <img src="assets/cube.gif" style="flex: 1; object-fit: cover; max-height: 200px;">
  <img src="assets/simulation.gif" style="flex: 1; object-fit: cover; max-height: 200px;">
</div>

### 📋 Content

- [Features](#-features)
- [Usage](#-usage)
  - [Examples](#examples)
- [Requirements & Dependencies](#-requirements--dependencies)
- [Building](#-building)
  - [Conan](#2a-via-conan-package-manager)
  - [Manual](#2b-via-manual-package-installation)
- [Documentation](#-documentation)
- [TODO List](TODO.md)

### 🔥 Features

- **Vulkan Sync2 Ready**: Uses modern synchronization tools
- **Stateless Execution**: The graph is compiled once; transitions are cached as 
lightweight templates, resulting in near-zero CPU overhead during the execution
- **Dynamic Resource Association**: Opaque virtual IDs are mapped to physical resources
at runtime for transient resource aliasing
- **O(1) Access:** Fast indexing using lightweight virtual resource's IDs
- **Exception-Free**: Designed for the best performance in game engines
- **Zero Dependencies**: The core library depends on Vulkan SDK only
- **Docs**: The project written with clear **doxygen** documentation 

### 🚀 Usage

1. **Setup Phase**: Initialize your resources, declare opaque handles, and create pass nodes
    ```c++
    vk::Device device;
    uint32_t queueFamily;
    
    vk::Buffer vertices;
    vk::DeviceSize verticesSize;
    
    vk::Image renderTarget;
    vk::ImageView renderTargetView;
    
    // ^^^ initialize your Vulkan objects ^^^
    
    caldera::RenderGraph graph 
        { device, queueFamily, queueFamily, queueFamily };
    
    auto verticesID = graph.declare_buffer(verticesSize);
    auto targetID = graph.declare_texture(vk::ImageAspectFlagBits::eColor);
    
    caldera::PassNode drawPass { "Your draw pass name", caldera::QueueType::graphics };
    drawPass.read(verticesID, caldera::BufferUsage::vertex);
    drawPass.write(targetID, caldera::TextureUsage::color_attachment);
    drawPass.callback([] (vk::CommandBuffer) { /* your draw commands */ });
    
    caldera::PassNode presentPass { "Present Pass", caldera::QueueType::graphics };
    presentPass.read(targetID, caldera::TextureUsage::present);
    presentPass.callback([] (vk::CommandBuffer) { ... });
    ```

2. **Compilation Phase**: Add your passes to the graph and compile it. The graph analyzes access
hazards (RAW, WAW, WAR) and generates barriers automatically

    ```c++
    // push passes to the graph
    graph.push_pass(std::move(drawPass));
    graph.push_pass(std::move(presentPass));
    
    // compile the graph
    graph.compile();
    ```

3. **Execution Phase**: Inside your frame loop, dynamically associate raw Vulkan handles 
with virtual IDs and record commands

    ```c++
    
    // associate resources to opaque handles
    graph.associate(verticesID, vertices);
    graph.associate(targetID, renderTarget, renderTargetView);
    
    // execute the graph
    vk::CommandBuffer cmd;
    cmd.begin(vk::CommandBufferBeginInfo{});
    
    graph.execute(cmd);
    
    cmd.end();
    
    ```

> Since the graph architecture is completely stateless, `graph.associate(...)` 
> **doesn't require** recompilation of the graph

#### Examples

- **Rotating cube**:
  simplest demo of the Render Graph Usage. Includes the following graphs:
  - Model loading: Staging Pass → Transfer Pass
  - Rendering: Draw Pass → Present Pass
  
- **Simulation**:
  simulates the gravity between **128** bodies on **GPU** via compute shaders, then draws 
  processed planets. Here is graph usages:
  - Planets loading: Staging Pass → Transfer Pass
  - Frame graph: Physics Pass → Draw pass → Present Pass

### 🛠️ Requirements & Dependencies

- C++20 compiler
- CMake (3.25+)
- Vulkan SDK (1.4+)
- Conan (2.0+)

For documentations:

- [Doxygen](https://www.doxygen.nl/)
- [Graphviz](https://graphviz.org/)
- [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css/tree/main) -
Improves appearance of docs (goes as a git submodule)

### 3rd Party Software

- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)

Used by the *example application* **only**:

- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- [GLFW](https://github.com/glfw/glfw)
- [spdlog](https://github.com/gabime/spdlog)
- [glm](https://github.com/g-truc/glm)
- [stb_image](https://github.com/nothings/stb)
- [shader-link](https://github.com/seweex/shader-link) (goes as a git submodule)

## ⚙️ Building

### 0. Install Vulkan SDK

Go to [official Vulkan SDK source](https://vulkan.lunarg.com/sdk/home) and install it

### 1. Clone the repository:

```bash
git clone --recursive https://github.com/seweex/caldera-render-graph.git
cd caldera-render-graph
```

### 2a. Via `conan` package manager

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

## 📄 Documentation

You can easily configure the docs via `doxygen`:

```bash
doxygen Doxyfile
```

See for the generated docs in the [doc/html/index.html](doc/html/index.html). 
Open it with your browser