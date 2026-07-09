#ifndef CALDERA_GRAPH_H
#define CALDERA_GRAPH_H

#include <config.h>
#include <resource.h>

namespace caldera
{
    class RenderGraph
    {
        [[nodiscard]] uint32_t choose_family(QueueType type) const noexcept;
        [[nodiscard]] detail::TextureVulkanState gen_low_level_state(detail::TextureState state) const;
        [[nodiscard]] detail::BufferVulkanState gen_low_level_state(detail::BufferState state) const;

    public:
        RenderGraph(
            vk::Device device,
            uint32_t graphicsFamily,
            uint32_t transferFamily,
            uint32_t computeFamily);

        RenderGraph(RenderGraph&&) noexcept = default;
        RenderGraph& operator=(RenderGraph&&) noexcept = default;

        RenderGraph(RenderGraph const&) = delete;
        RenderGraph& operator=(RenderGraph const&) = delete;

        [[nodiscard]] TextureID declare_texture(vk::ImageAspectFlags aspects);
        [[nodiscard]] BufferID declare_buffer(vk::DeviceSize size);

        void associate(TextureID id, vk::Image image, vk::ImageView view);
        void associate(BufferID id, vk::Buffer buffer);

        void push_pass(PassNode pass);

        void compile();
        void execute(vk::CommandBuffer cmd);

    private:
        vk::Device m_device;

        uint32_t m_graphicsFamily;
        uint32_t m_transferFamily;
        uint32_t m_computeFamily;

        std::vector<PassNode> m_passes;

        std::vector<detail::TextureDescription> m_textures;
        std::vector<detail::BufferDescription> m_buffers;
    };
}

#endif
