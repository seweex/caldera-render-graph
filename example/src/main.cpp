
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
    std::queue<uint64_t> ticketsQueue;

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

            firstFrame = false;
        }

        auto cmd = sch.get_current_command_buffer();
        cmd.begin(vk::CommandBufferBeginInfo{});

        vk::RenderingAttachmentInfo attachmentInfo;
        attachmentInfo.imageView = swp.imageViews[sch.currentImage];
        attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
        attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
        attachmentInfo.clearValue = vk::ClearColorValue{ 1.f, 1.f, 0.2f, 0.f };

        vk::ImageMemoryBarrier2 const toWriteBarrier{
            vk::PipelineStageFlagBits2::eAllCommands,
            vk::AccessFlagBits2::eNone,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::QueueFamilyIgnored,
            vk::QueueFamilyIgnored,
            swp.images[sch.currentImage],
            vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        };

        cmd.pipelineBarrier2(vk::DependencyInfo{
            vk::DependencyFlags{},
            0, nullptr,
            0, nullptr,
            1, &toWriteBarrier
        });

        cmd.beginRendering(vk::RenderingInfo {
            vk::RenderingFlags{},
            vk::Rect2D{ vk::Offset2D{ 0, 0 }, vk::Extent2D{ 1024, 576 } },
            1, 0,
            attachmentInfo
        });

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, ppl.pipeline);

        uint64_t constexpr offset = 0;
        cmd.bindVertexBuffers(0, cubeVertices.buffer, offset);
        cmd.bindIndexBuffer(cubeIndices.buffer, 0, vk::IndexType::eUint32);

        cmd.endRendering();

        vk::ImageMemoryBarrier2 const toPresentBarrier{
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::PipelineStageFlagBits2::eNone,
            vk::AccessFlagBits2::eNone,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::QueueFamilyIgnored,
            vk::QueueFamilyIgnored,
            swp.images[sch.currentImage],
            vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        };

        cmd.pipelineBarrier2(vk::DependencyInfo{
            vk::DependencyFlags{},
            0, nullptr,
            0, nullptr,
            1, &toPresentBarrier
        });

        cmd.end();

        auto const ticket = sch.submit_current_buffer(0, true);

        if (ticket == 0 ||
            !sch.end_frame())
            return 1;

        ticketsQueue.push(ticket);
        if (ticketsQueue.size() > 3)
            ticketsQueue.pop();
    }

    while (!ticketsQueue.empty()) {
        sch.wait_for_ticket(ticketsQueue.front());
        ticketsQueue.pop();
    }

    return 0;
}
