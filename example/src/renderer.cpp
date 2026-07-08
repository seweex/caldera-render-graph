
#include <renderer.h>
#include <mesh.h>

namespace caldera_example
{
    void Renderer::begin()
    {
        vk::ImageMemoryBarrier2 const barrierInfo
        {
            vk::PipelineStageFlagBits2::eAllCommands,
            vk::AccessFlagBits2::eNone,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::QueueFamilyIgnored,
            vk::QueueFamilyIgnored,
            image,
            vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        };

        vk::DependencyInfo dependencyInfo;
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &barrierInfo;

        vk::RenderingAttachmentInfo attachmentInfo;
        attachmentInfo.imageView = view;
        attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
        attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
        attachmentInfo.clearValue = vk::ClearColorValue{ 0.f, 0.f, 0.f, 0.f };

        vk::RenderingInfo renderingInfo;
        renderingInfo.renderArea = vk::Rect2D{ vk::Offset2D{ 0, 0 }, vk::Extent2D{ 1024, 576 } };
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &attachmentInfo;
        renderingInfo.layerCount = 1;

        commandBuffer.pipelineBarrier2(dependencyInfo);
        commandBuffer.beginRendering(renderingInfo);
    }

    void Renderer::end()
    {
        vk::ImageMemoryBarrier2 const barrierInfo
        {
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::PipelineStageFlagBits2::eNone,
            vk::AccessFlagBits2::eNone,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::QueueFamilyIgnored,
            vk::QueueFamilyIgnored,
            image,
            vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        };

        vk::DependencyInfo dependencyInfo;
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &barrierInfo;

        commandBuffer.endRendering();
        commandBuffer.pipelineBarrier2(dependencyInfo);
    }

    void Renderer::bind_mesh(vk::Buffer vertices, vk::Buffer indices)
    {
        constexpr uint64_t offset = 0;

        commandBuffer.bindVertexBuffers(0, vertices, offset);
        commandBuffer.bindIndexBuffer(indices, 0, vk::IndexType::eUint32);
    }

    void Renderer::bind_material(vk::Pipeline pipeline) {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    }

    void Renderer::draw() {
        commandBuffer.drawIndexed(caldera_example::cube_indices.size(), 1, 0, 0, 0);
    }
}
