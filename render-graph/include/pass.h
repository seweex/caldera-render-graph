#ifndef CALDERA_PASS_H
#define CALDERA_PASS_H

#include <config.h>

#include <string>
#include <vector>
#include <functional>

namespace caldera
{
    enum class QueueType
    {
        none,
        graphics,
        transfer,
        compute
    };

    class PassNode
    {
    public:
        PassNode(std::string_view name, QueueType type);

        PassNode(PassNode&&) noexcept = default;
        PassNode& operator=(PassNode&&) noexcept = default;

        PassNode(PassNode const&) = default;
        PassNode& operator=(PassNode const&) = default;

        void write(TextureID texture, TextureUsage usage);
        void write(BufferID buffer, BufferUsage usage);

        void read(TextureID texture, TextureUsage usage);
        void read(BufferID buffer, BufferUsage usage);

        void callback(std::function<void(vk::CommandBuffer)> func);

        [[nodiscard]] bool is_completed() const noexcept;

    private:
        friend ::caldera::RenderGraph;

        void reset_barriers();

        std::string m_name;
        QueueType m_queueType;

        std::vector<detail::TextureBinding> m_textureBindings;
        std::vector<detail::BufferBinding> m_bufferBindings;

        std::function<void(vk::CommandBuffer)> m_callback;

        std::vector<detail::TextureTransition> m_textureTransitions;
        std::vector<detail::BufferTransition> m_bufferTransitions;

        std::vector<vk::ImageMemoryBarrier2> m_imageBarriers;
        std::vector<vk::BufferMemoryBarrier2> m_bufferBarriers;
    };
}

#endif
