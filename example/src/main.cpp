
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <spdlog/spdlog.h>

#include <queue>
#include <window.h>
#include <device.h>
#include <swapchain.h>
#include <shader.h>
#include <descriptor.h>
#include <memory.h>
#include <image.h>
#include <buffer.h>
#include <scheduler.h>
#include <pipeline.h>
#include <mesh.h>
#include <renderer.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shaders/basic.frag.hpp>
#include <shaders/basic.vert.hpp>

#include <graph.h>
#include <pass.h>

int main()
{
    spdlog::set_level(spdlog::level::info);

    caldera_example::Context ctx;
    caldera_example::Window wnd;
    caldera_example::Device dvc;
    caldera_example::Swapchain swp;
    caldera_example::Scheduler sch;

    caldera_example::Shader vsh;
    caldera_example::Shader fsh;

    caldera_example::BindlessLayout lyt;
    caldera_example::BindlessDescriptors dsc;

    caldera_example::Allocator alc;
    caldera_example::Pipeline ppl;

    caldera_example::Buffer cubeVertices;
    caldera_example::Buffer cubeIndices;
    caldera_example::Buffer staging;

    if (!ctx.init() ||
        !wnd.init(ctx) ||
        !dvc.init(ctx, wnd) ||
        !swp.init(dvc, wnd) ||
        !sch.init(dvc, swp, 3) ||
        !vsh.init(dvc, shader_link_compiled::spv_basic_vert) ||
        !fsh.init(dvc, shader_link_compiled::spv_basic_frag) ||
        !lyt.init(dvc) ||
        !dsc.init(dvc, lyt) ||
        !alc.init(ctx, dvc) ||
        !staging.init(dvc, alc, { sizeof(caldera_example::cube_vertices) + sizeof(caldera_example::cube_indices), caldera_example::Buffer::MemoryType::constantly_mapped, vk::BufferUsageFlagBits::eTransferSrc }) ||
        !cubeVertices.init(dvc, alc, { sizeof(caldera_example::cube_vertices), caldera_example::Buffer::MemoryType::gpu_local, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst }) ||
        !cubeIndices.init(dvc, alc, { sizeof(caldera_example::cube_indices), caldera_example::Buffer::MemoryType::gpu_local, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst }) ||
        !ppl.init(wnd, dvc, lyt, vsh, fsh))
    {
        return 1;
    }

    caldera::RenderGraph graph{ dvc.device, dvc.queueFamilyIndex, dvc.queueFamilyIndex, dvc.queueFamilyIndex };

    auto const stagingID = graph.declare_buffer(sizeof(caldera_example::cube_vertices) + sizeof(caldera_example::cube_indices));
    auto const cubeVerticesID = graph.declare_buffer(sizeof(caldera_example::cube_vertices));
    auto const cubeIndicesID = graph.declare_buffer(sizeof(caldera_example::cube_indices));

    auto const swapchainImageID = graph.declare_texture(vk::ImageAspectFlagBits::eColor);

    caldera::PassNode stagingPass{ "staging-fill-pass", caldera::QueueType::transfer };
    stagingPass.write(stagingID, caldera::BufferUsage::mapped_usage);

    caldera::PassNode meshPass{ "mesh-copy-pass", caldera::QueueType::transfer };
    meshPass.read(stagingID, caldera::BufferUsage::transfer);
    meshPass.write(cubeVerticesID, caldera::BufferUsage::transfer);
    meshPass.write(cubeIndicesID, caldera::BufferUsage::transfer);

    caldera::PassNode mainPass { "main-pass", caldera::QueueType::graphics };
    mainPass.read(cubeVerticesID, caldera::BufferUsage::vertex);
    mainPass.read(cubeIndicesID, caldera::BufferUsage::index);
    mainPass.write(swapchainImageID, caldera::TextureUsage::color_attachment);

    caldera::PassNode presentPass { "present-pass", caldera::QueueType::graphics };
    presentPass.read(swapchainImageID, caldera::TextureUsage::present);

    void* stagingMapping;
    uint32_t frameCounter = 0;
    uint32_t prevTime = 0;

    stagingPass.callback([&] (vk::CommandBuffer const cmd)
        {
            std::memcpy(stagingMapping, caldera_example::cube_vertices.data(), sizeof(caldera_example::cube_vertices));
            auto nextStagingMapping = static_cast<std::byte*>(stagingMapping) + sizeof(caldera_example::cube_vertices);
            std::memcpy(nextStagingMapping, caldera_example::cube_indices.data(), sizeof(caldera_example::cube_indices));
        });

    meshPass.callback([&] (vk::CommandBuffer const cmd)
        {
            vk::BufferCopy2 constexpr verticesRegion { 0, 0, sizeof(caldera_example::cube_vertices) };
            cmd.copyBuffer2(vk::CopyBufferInfo2{ staging.buffer, cubeVertices.buffer, verticesRegion });

            vk::BufferCopy2 constexpr indicesRegion { sizeof(caldera_example::cube_vertices), 0, sizeof(caldera_example::cube_indices) };
            cmd.copyBuffer2(vk::CopyBufferInfo2{ staging.buffer, cubeIndices.buffer, indicesRegion });
        });

    mainPass.callback([&] (vk::CommandBuffer const cmd)
        {
            caldera_example::Renderer rdr{
                swp.images[sch.currentImage], swp.imageViews[sch.currentImage], cmd };

            rdr.bind_mesh(cubeVertices.buffer, cubeIndices.buffer);
            rdr.bind_material(ppl.pipeline);

            glm::mat4 model = glm::rotate(
                glm::mat4(1.0f),
                static_cast<float>(glfwGetTime()) * glm::radians(45.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

            glm::vec3 constexpr target { 0.0f, 0.0f, 0.0f };
            glm::vec3 constexpr eye = target + glm::vec3(4.5f);
            glm::vec3 constexpr up{ 0.0f, 1.0f, 0.0f };

            glm::mat4 const view = glm::lookAt(eye, target, up);

            auto constexpr aspect = static_cast<float>(1024) / static_cast<float>(576);
            glm::mat4 proj = glm::perspective(glm::radians(35.0f), aspect, 0.1f, 1000.0f);

            proj[1][1] *= -1.0f;

            glm::mat4 mvpMatrix = proj * view * model;

            rdr.push_constant(lyt.pipelineLayout, mvpMatrix);
            rdr.begin();
            rdr.draw();
            rdr.end();

            ++frameCounter;

            auto const currTime = static_cast<uint32_t>(glfwGetTime());

            if (currTime > prevTime)
            {
                prevTime = currTime;
                spdlog::info("FPS: {}", frameCounter);
                frameCounter = 0;
            }
        });

    presentPass.callback([] (vk::CommandBuffer) {});

    graph.push_pass(std::move(stagingPass));
    graph.push_pass(std::move(meshPass));
    graph.push_pass(std::move(mainPass));
    graph.push_pass(std::move(presentPass));

    graph.compile();

    stagingMapping = staging.get_constant_mapping();

    graph.associate(stagingID, staging.buffer);
    graph.associate(cubeVerticesID, cubeVertices.buffer);
    graph.associate(cubeIndicesID, cubeIndices.buffer);

    while (!wnd.closing())
    {
        caldera_example::Window::poll_events();

        if (!sch.begin_frame())
            return 1;

        graph.associate(swapchainImageID, swp.images[sch.currentImage], swp.imageViews[sch.currentImage] );

        auto cmd = sch.get_current_command_buffer();
        cmd.begin(vk::CommandBufferBeginInfo{});

        graph.execute(cmd);

        cmd.end();

        auto const ticket = sch.submit_current_buffer(0, true);

        if (ticket == 0 ||
            !sch.end_frame())
            return 1;
    }

    return  sch.wait_idle() ? 0 : 1;
}
