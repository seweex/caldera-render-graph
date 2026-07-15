# Caldera – Render Graph

A lightweight and easily extensible library for automatic management of Vulkan barriers and resources

<p align="left">
  <img src="assets/cube.gif" height="200" alt="Cube" />
  <img src="assets/simulation.gif" height="200" alt="Simulation" />
</p>

## 📋 Content

- [Features](#-features)
- [3rd Party Solutions](#-3rd-party-solutions)
- [Usage](#-usage)
  - [Examples](#examples)
    - [Rotating Cube](examples/rotating-cube/README.md)
    - [Simulation](examples/simulation/README.md)
- [Requirements & Dependencies](#-requirements--dependencies)
  - [Basic requirements & deps](#basic-requirements--deps)
  - [Example's requirements](#examples-requirements)
- [Building](#-building)
  - [Step-by-step guide](#1-clone-the-repository)
  - [CMake Targets](#cmake-targets)
  - [CMake Options](#cmake-options)
- [Documentation](#-documentation)
- [TODO List](TODO.md)

## 🔥 Features

- **Vulkan Sync2 Ready**: Uses modern synchronization tools
- **Stateless Execution**: The graph is compiled once; transitions are cached as 
lightweight templates, resulting in near-zero CPU overhead during the execution
- **Dynamic Resource Association**: Opaque virtual IDs are mapped to physical resources
at runtime for transient resource aliasing
- **O(1) Access:** Fast indexing using lightweight virtual resource's IDs
- **Exception-Free**: Designed for the best performance in game engines
- **Zero Dependencies**: The core library depends on Vulkan SDK and VMA only
- **Docs**: The project written with clear **doxygen** documentation 

## 🔩 3rd Party Solutions

Thanks to developers for this best products:

- [Vulkan](https://vulkan.lunarg.com/sdk/home)
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- [GLFW](https://github.com/glfw/glfw)
- [spdlog](https://github.com/gabime/spdlog)
- [glm](https://github.com/g-truc/glm)
- [Doxygen](https://www.doxygen.nl/)
- [Graphviz](https://graphviz.org/)
- [stb_image](https://github.com/nothings/stb)

And also there is my another project 

- [shader-link](https://github.com/seweex/shader-link) - python script for shader compilations

## 🚀 Usage

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

### Examples

- **Rotating cube**:
  simplest demo of the Render Graph Usage. Includes the following graphs:
  - Model loading: Staging Pass → Transfer Pass
  - Rendering: Draw Pass → Present Pass
  
- **Simulation**:
  simulates the gravity between **128** bodies on **GPU** via compute shaders, then draws 
  processed planets. Here is graph usages:
  - Planets loading: Staging Pass → Transfer Pass
  - Frame graph: Physics Pass → Draw pass → Present Pass

## 🛠️ Requirements & Dependencies

### Basic requirements & deps

There is tools & packages you need to install manually. 
[Here]() is a list of all used 3rd party solutions.

Tools:
- C++20 compiler
- CMake (3.25+)
- Conan (2.0+) - for examples **only**

Dependencies
- [Vulkan SDK (1.4+)](https://vulkan.lunarg.com/sdk/home)

_For docs_:
- [Doxygen](https://www.doxygen.nl/)
- [Graphviz](https://graphviz.org/)

### Example's requirements

3rd party solutions used by example apps only:

- [Python Interpreter](https://python.com)


## ⚙️ Building

### 0. Install required deps

Install all needed dependencies listed [here](#-requirements--dependencies)

### 1. Clone the repository

```bash
git clone https://github.com/seweex/caldera-render-graph.git
cd caldera-render-graph
```

### 2. Configure the project

Specify your generator and CMake variables
> It's **required** to specify the `CMAKE_BUILD_TYPE` (Debug/Release) variable on
> the configuration step

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -G Ninja
```

### 3. Build required targets

```bash
cmake --build . --target=caldera-render-graph
```

### CMake Targets
Targets declared by the project:
- `caldera-render-graph` - the main library target
- `caldera-rotating-cube` - the [rotating cube](examples/rotating-cube/README.md) target, required `CALDERA_BUILD_EXAMPLES=ON`
- `caldera-simulation` - the [simulation](examples/simulation/README.md) target, required `CALDERA_BUILD_EXAMPLES=ON`
- `caldera-docs` - generates docs for the graph, required `CALDERA_GEN_DOCS=ON`

### CMake Options
- `CALDERA_BUILD_EXAMPLES` - enables example building, default is **False**
- `CALDERA_USE_SANITIZERS` - enables building with sanitizers, default is **CMAKE_BUILD_TYPE==Debug**
- `CALDERA_GEN_DOCS` - enables docs generation target, default is **False**

## 📄 Documentation

Before generation docs, install docs-required [dependencies](#-requirements--dependencies)

You can easily configure the docs for the graph. To gen docs, configure 
the CMake project with `-DCALDERA_GEN_DOCS=ON` and build the `caldera-docs` target.

Then see for the generated docs in the [docs/html/index.html](docs/html/index.html). 
Open it with your browser
