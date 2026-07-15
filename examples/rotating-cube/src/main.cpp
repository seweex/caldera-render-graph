
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <spdlog/spdlog.h>
#include <stb_image.h>

#include <caldera-examples-common/window.h>
#include <caldera-examples-common/device.h>
#include <caldera-examples-common/swapchain.h>
#include <caldera-examples-common/shader.h>
#include <caldera-examples-common/descriptor.h>
#include <caldera-examples-common/memory.h>
#include <caldera-examples-common/image.h>
#include <caldera-examples-common/buffer.h>
#include <caldera-examples-common/scheduler.h>
#include <caldera-examples-common/pipeline.h>
#include <caldera-examples-common/mesh.h>
#include <caldera-examples-common/renderer.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <compiled-shaders/basic.frag.hpp>
#include <compiled-shaders/basic.vert.hpp>

#include <caldera-render-graph/graph.h>
#include <caldera-render-graph/pass.h>

using namespace caldera;
using namespace caldera_examples_common;

struct Graphics
{
    Context context;
    Window window;
    Device device;
    Swapchain swapchain;
    Scheduler scheduler;

    BindlessLayout layout;
    BindlessDescriptors descriptors;

    Pipeline pipeline;
    Allocator allocator;
};

struct Resources
{
    Shader vertexShader;
    Shader fragmentShader;

    Buffer stagingBuffer;
    Buffer cubeVertices;
    Buffer cubeIndices;
};

bool initialize(Graphics& graphics, Resources& resources)
{
    Buffer::Settings constexpr vertices_settings
        { sizeof(cube_vertices), Buffer::MemoryType::gpu_local, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst };

    Buffer::Settings constexpr indices_settings
        { sizeof(cube_indices), Buffer::MemoryType::gpu_local, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst };

    Buffer::Settings constexpr staging_settings
        { vertices_settings.size + indices_settings.size, Buffer::MemoryType::constantly_mapped, vk::BufferUsageFlagBits::eTransferSrc };

    if (!graphics.context.init() ||
        !graphics.window.init(graphics.context) ||
        !graphics.device.init(graphics.context, graphics.window) ||
        !graphics.swapchain.init(graphics.device, graphics.window) ||
        !graphics.scheduler.init(graphics.device, graphics.swapchain, 1) ||
        !graphics.layout.init(graphics.device) ||
        !graphics.descriptors.init(graphics.device, graphics.layout) ||
        !graphics.allocator.init(graphics.context, graphics.device) ||
        !resources.vertexShader.init(graphics.device, shader_link_compiled::spv_basic_vert) ||
        !resources.fragmentShader.init(graphics.device, shader_link_compiled::spv_basic_frag) ||
        !resources.stagingBuffer.init(graphics.device, graphics.allocator, staging_settings) ||
        !resources.cubeVertices.init(graphics.device, graphics.allocator, vertices_settings) ||
        !resources.cubeIndices.init(graphics.device, graphics.allocator, indices_settings) ||
        !graphics.pipeline.init(graphics.window, graphics.device, graphics.layout, resources.vertexShader, resources.fragmentShader))
    {
        return false;
    }

    return true;
}

RenderGraph make_load_graph(
    Graphics const& graphics, Resources& resources)
{
    RenderGraph graph {
        graphics.device.device,
        graphics.device.queueFamilyIndex,
        graphics.device.queueFamilyIndex,
        graphics.device.queueFamilyIndex,
    };

    auto const stagingBufferID = graph.declare_buffer(sizeof(cube_vertices) + sizeof(cube_indices));
    auto const cubeVerticesID = graph.declare_buffer(sizeof(cube_vertices));
    auto const cubeIndicesID = graph.declare_buffer(sizeof(cube_indices));

    graph.associate(stagingBufferID, resources.stagingBuffer.buffer);
    graph.associate(cubeVerticesID, resources.cubeVertices.buffer);
    graph.associate(cubeIndicesID, resources.cubeIndices.buffer);

    PassNode stagingPass { "staging-pass", QueueType::transfer };
    stagingPass.write(stagingBufferID, BufferUsage::mapped_usage);
    stagingPass.callback([&] (vk::CommandBuffer)
        {
            auto mapping = static_cast<std::byte*>(resources.stagingBuffer.get_constant_mapping());
            std::memcpy(mapping, cube_vertices.data(), sizeof(cube_vertices));

            mapping += sizeof(cube_vertices);
            std::memcpy(mapping, cube_indices.data(), sizeof(cube_indices));
        });

    PassNode transferPass { "transfer-pass", QueueType::transfer };
    transferPass.read(stagingBufferID, BufferUsage::transfer);
    transferPass.write(cubeVerticesID, BufferUsage::transfer);
    transferPass.write(cubeIndicesID, BufferUsage::transfer);
    transferPass.callback([&] (vk::CommandBuffer const cmd)
        {
            vk::BufferCopy2 constexpr verticesRegion { 0, 0, sizeof(cube_vertices) };
            vk::CopyBufferInfo2 const verticesInfo { resources.stagingBuffer.buffer, resources.cubeVertices.buffer, verticesRegion };

            vk::BufferCopy2 constexpr indicesRegion { sizeof(cube_vertices), 0, sizeof(cube_indices) };
            vk::CopyBufferInfo2 const indicesInfo { resources.stagingBuffer.buffer, resources.cubeIndices.buffer, indicesRegion };

            cmd.copyBuffer2(verticesInfo);
            cmd.copyBuffer2(indicesInfo);
        });

    graph.push_pass(std::move(stagingPass));
    graph.push_pass(std::move(transferPass));

    graph.compile();
    return graph;
}

std::pair<RenderGraph, TextureID> make_draw_graph(
    Graphics const& graphics, Resources const& resources)
{
    RenderGraph graph {
        graphics.device.device,
        graphics.device.queueFamilyIndex,
        graphics.device.queueFamilyIndex,
        graphics.device.queueFamilyIndex,
    };

    auto const cubeVerticesID = graph.declare_buffer(sizeof(cube_vertices));
    auto const cubeIndicesID = graph.declare_buffer(sizeof(cube_indices));
    auto const swapchainImageID = graph.declare_texture(vk::ImageAspectFlagBits::eColor);

    graph.associate(cubeVerticesID, resources.cubeVertices.buffer);
    graph.associate(cubeIndicesID, resources.cubeIndices.buffer);

    PassNode drawPass { "draw-pass", QueueType::graphics };
    drawPass.write(swapchainImageID, TextureUsage::color_attachment);
    drawPass.read(cubeVerticesID, BufferUsage::vertex);
    drawPass.read(cubeIndicesID, BufferUsage::index);
    drawPass.callback([&] (vk::CommandBuffer const cmd)
        {
            Renderer renderer {
                graphics.swapchain.images[graphics.scheduler.currentImage],
                graphics.swapchain.imageViews[graphics.scheduler.currentImage],
                cmd };

            renderer.bind_mesh(resources.cubeVertices.buffer, resources.cubeIndices.buffer);
            renderer.bind_material(graphics.pipeline.pipeline);

            auto const model = glm::rotate(
                glm::mat4 { 1.0f },
                static_cast<float>(glfwGetTime()) * glm::radians(45.0f),
                glm::vec3 {0.f, 1.f, 0.f} );

            auto const view = glm::lookAt(
                glm::vec3{ 6.f, 6.f, 6.f },
                glm::vec3{ 0.f, 0.f, 0.f },
                glm::vec3{ 0.f, 1.f, 0.f });

            auto proj = glm::perspective(
                glm::radians(35.f),
                1024.f / 576.f,
                0.1f, 1000.f);

            proj[1][1] *= -1.f;
            auto const matrix = proj * view * model;

            renderer.push_constant(graphics.layout.pipelineLayout, matrix);
            renderer.begin();
            renderer.draw();
            renderer.end();
        });

    PassNode presentPass { "present-pass", QueueType::graphics };
    presentPass.read(swapchainImageID, TextureUsage::present);
    presentPass.callback([] (vk::CommandBuffer) {});

    graph.push_pass(std::move(drawPass));
    graph.push_pass(std::move(presentPass));

    graph.compile();
    return std::make_pair(std::move(graph), swapchainImageID);
}

int main()
{
    spdlog::set_level(spdlog::level::info);

    Graphics graphics;
    Resources resources;

    if (!initialize(graphics, resources))
        return 1;

    auto loadGraph = make_load_graph(graphics, resources);
    auto [drawGraph, swapchainImageID] = make_draw_graph(graphics, resources);

    bool firstFrame = true;
    uint32_t frameCount = 0;
    uint32_t prevTime = 0;

    while (!graphics.window.closing())
    {
        Window::poll_events();

        if (!graphics.scheduler.begin_frame())
            return 1;

        drawGraph.associate(
            swapchainImageID,
            graphics.swapchain.images[graphics.scheduler.currentImage],
            graphics.swapchain.imageViews[graphics.scheduler.currentImage]);

        auto cmd = graphics.scheduler.get_current_command_buffer();

        if (auto const result = cmd.begin(vk::CommandBufferBeginInfo{});
            result < vk::Result::eSuccess)
        {
            spdlog::error("Failed to begin a command buffer: {}", vk::to_string(result));
            return 1;
        }

        if (firstFrame) {
            loadGraph.execute(cmd);
            firstFrame = false;
        }

        drawGraph.execute(cmd);

        if (auto const result = cmd.end();
            result < vk::Result::eSuccess)
        {
            spdlog::error("Failed to begin a command buffer: {}", vk::to_string(result));
            return 1;
        }

        if (0 == graphics.scheduler.submit_current_buffer(0, true) ||
            !graphics.scheduler.end_frame())
            return 1;

        ++frameCount;

        if (auto const currTime = static_cast<uint32_t>(glfwGetTime());
            currTime > prevTime)
        {
            prevTime = currTime;
            spdlog::info("FPS: {}", frameCount);
            frameCount = 0;
        }
    }

    return graphics.scheduler.wait_idle() ? 0 : 1;
}
