
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
        !staging.init(dvc, alc, { sizeof(caldera_example::cube_vertices), caldera_example::Buffer::MemoryType::constantly_mapped, vk::BufferUsageFlagBits::eTransferSrc }) ||
        !cubeVertices.init(dvc, alc, { sizeof(caldera_example::cube_vertices), caldera_example::Buffer::MemoryType::gpu_local, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst }) ||
        !cubeIndices.init(dvc, alc, { sizeof(caldera_example::cube_indices), caldera_example::Buffer::MemoryType::gpu_local, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst }) ||
        !ppl.init(wnd, dvc, lyt, vsh, fsh))
    {
        return 1;
    }

    bool firstFrame = true;

    while (!wnd.closing())
    {
        caldera_example::Window::poll_events();

        if (!sch.begin_frame())
            return 1;

        spdlog::info("Passing a frame");

        /* Copy mesh */
        if (firstFrame)
        {
            /* Vertices */

            auto cmd = sch.get_current_command_buffer();
            cmd.begin(vk::CommandBufferBeginInfo{});

            auto const stagingMapping = staging.get_constant_mapping();
            std::memcpy(stagingMapping, caldera_example::cube_vertices.data(), sizeof(caldera_example::cube_vertices));

            vk::BufferCopy2 const verticesRegion { 0, 0, sizeof(caldera_example::cube_vertices) };
            cmd.copyBuffer2(vk::CopyBufferInfo2{ staging.buffer, cubeVertices.buffer, verticesRegion });
            cmd.end();

            auto const verticesCopyTicket = sch.submit_current_buffer(0, false);
            sch.wait_for_ticket(verticesCopyTicket);

            /* Indices */

            cmd = sch.get_current_command_buffer();
            cmd.begin(vk::CommandBufferBeginInfo{});

            std::memcpy(stagingMapping, caldera_example::cube_indices.data(), sizeof(caldera_example::cube_indices));

            vk::BufferCopy2 const indicesRegion { 0, 0, sizeof(caldera_example::cube_indices) };
            cmd.copyBuffer2(vk::CopyBufferInfo2{ staging.buffer, cubeIndices.buffer, indicesRegion });
            cmd.end();

            auto const indicesCopyTicket = sch.submit_current_buffer(0, false);
            sch.wait_for_ticket(indicesCopyTicket);
        }

        auto cmd = sch.get_current_command_buffer();
        cmd.begin(vk::CommandBufferBeginInfo{});

        caldera_example::Renderer rdr{
            swp.images[sch.currentImage], swp.imageViews[sch.currentImage], cmd };

        rdr.bind_mesh(cubeVertices.buffer, cubeIndices.buffer);
        rdr.bind_material(ppl.pipeline);

        glm::mat4 model = glm::rotate(
            glm::mat4(1.0f),
            static_cast<float>(glfwGetTime()) * glm::radians(45.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));

        glm::vec3 const target { 0.0f, 0.0f, 0.0f };
        glm::vec3 const eye = target + glm::vec3(4.5f);
        glm::vec3 const up{ 0.0f, 1.0f, 0.0f };

        glm::mat4 view = glm::lookAt(eye, target, up);

        float const aspect = static_cast<float>(1024) / static_cast<float>(576);
        glm::mat4 proj = glm::perspective(glm::radians(35.0f), aspect, 0.1f, 1000.0f);

        proj[1][1] *= -1.0f;

        glm::mat4 const mvpMatrix = proj * view * model;

        rdr.push_constant(lyt.pipelineLayout, mvpMatrix);

        rdr.begin();

        rdr.draw();

        rdr.end();
        cmd.end();

        auto const ticket = sch.submit_current_buffer(0, true);

        if (ticket == 0 ||
            !sch.end_frame())
            return 1;

        firstFrame = false;
    }

    return  sch.wait_idle() ? 0 : 1;
}
