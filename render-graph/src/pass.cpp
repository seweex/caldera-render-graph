
#include <caldera-render-graph/pass.h>
#include <caldera-render-graph/resource.h>

namespace caldera
{
    PassNode::PassNode(std::string_view const name, QueueType const type) :
        m_name(name),
        m_queueType(type)
    {}

    void PassNode::write(TextureID const texture, TextureUsage const usage) {
        m_textureBindings.emplace_back(texture,
            detail::TextureState{ m_queueType, usage, detail::ResourceAccess::write });
    }

    void PassNode::write(BufferID const buffer, BufferUsage const usage) {
        m_bufferBindings.emplace_back(buffer,
            detail::BufferState{ m_queueType, usage, detail::ResourceAccess::write });
    }

    void PassNode::read(TextureID const texture, TextureUsage const usage) {
        m_textureBindings.emplace_back(texture,
            detail::TextureState{ m_queueType, usage, detail::ResourceAccess::read });
    }

    void PassNode::read(BufferID const buffer, BufferUsage const usage) {
        m_bufferBindings.emplace_back(buffer,
            detail::BufferState{ m_queueType, usage, detail::ResourceAccess::read });
    }

    void PassNode::callback(std::function<void(vk::CommandBuffer)> func) {
        m_callback = std::move(func);
    }

    bool PassNode::is_completed() const noexcept {
        return static_cast<bool>(m_callback);
    }
}
