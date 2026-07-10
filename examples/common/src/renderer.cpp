
#include <renderer.h>
#include <../../common/include/mesh.h>

#include <glm/glm.hpp>

namespace caldera_example
{
    void Renderer::begin()
    {
        vk::RenderingAttachmentInfo attachmentInfo;
        attachmentInfo.imageView = view;
        attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
        attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
        attachmentInfo.clearValue = vk::ClearColorValue{ 0.8f, 0.6f, 0.7f, 0.f };

        vk::RenderingInfo renderingInfo;
        renderingInfo.renderArea = vk::Rect2D{ vk::Offset2D{ 0, 0 }, vk::Extent2D{ 1024, 576 } };
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &attachmentInfo;
        renderingInfo.layerCount = 1;

        commandBuffer.beginRendering(renderingInfo);
    }

    void Renderer::end() {
        commandBuffer.endRendering();
    }

    void Renderer::bind_mesh(vk::Buffer const vertices, vk::Buffer const indices)
    {
        constexpr uint64_t offset = 0;

        commandBuffer.bindVertexBuffers(0, vertices, offset);
        commandBuffer.bindIndexBuffer(indices, 0, vk::IndexType::eUint32);
    }

    void Renderer::bind_material(vk::Pipeline const pipeline) {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    }

    void Renderer::push_constant(vk::PipelineLayout const layout, glm::mat4 const& matrix)
    {
        commandBuffer.pushConstants(
            layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &matrix);
    }

    void Renderer::draw() {
        commandBuffer.drawIndexed(caldera_example::cube_indices.size(), 1, 0, 0, 0);
    }
}
