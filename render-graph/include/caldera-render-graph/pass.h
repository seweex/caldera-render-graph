#ifndef CALDERA_RENDER_GRAPH_PASS_H
#define CALDERA_RENDER_GRAPH_PASS_H

#include <caldera-render-graph/config.h>

#include <string>
#include <vector>
#include <functional>

namespace caldera
{
    /// @brief Type of queue family for which the Render Graph Pass is executed
    enum class QueueType
    {
        /**
         * @brief Specifies undefined queue family
         * @warning You should not use this value
         */
        none,

        /**
         * @brief Specifies the graphic queue family
         * Use it for graphics commands and present passes
         */
        graphics,

        /**
         * @brief Specifies the transfer queue family
         * Use it for transfer operations and mapped writing/reading
         */
        transfer,

        /**
         * @brief Specifies the compute queue family
         * Use it for compute passes
         */
        compute
    };

    /**
     * @brief The Render Graph Pass Node class
     * @note For detailed usage see the example/src/main.cpp file
     */
    class PassNode
    {
    public:
        /**
         * @brief Creates a new Pass Node
         *
         * @param name Name of the pass
         * @param type Type of queue family
         */
        PassNode(std::string_view name, QueueType type);

        PassNode(PassNode&&) noexcept = default;
        PassNode& operator=(PassNode&&) noexcept = default;

        PassNode(PassNode const&) = default;
        PassNode& operator=(PassNode const&) = default;

        /**
         * @brief Specifies, that this Pass will write to this texture
         *
         * @param texture Virtual texture ID for writing
         * @param usage Type of the passed texture usage
         *
         * @note texture must be given you from the Render Graph to which you will push this Pass Node
         * @note usage must be valid for the Pass queue family type
         * @note usage must not conflict with the previous usages of this texture in the whole program
         */
        void write(TextureID texture, TextureUsage usage);

        /**
         * @brief Specifies, that this Pass will write to this buffer
         *
         * @param buffer Virtual buffer ID for writing
         * @param usage Type of the passed buffer usage
         *
         * @note buffer must be given you from the Render Graph to which you will push this Pass Node
         * @note usage must be valid for the Pass queue family type
         * @note usage must not conflict with the previous usages of this buffer in the whole program
         */
        void write(BufferID buffer, BufferUsage usage);

        /**
         * @brief Specifies, that this Pass will read this texture
         *
         * @param texture Virtual texture ID for reading
         * @param usage Type of the passed texture usage
         *
         * @note texture must be given you from the Render Graph to which you will push this Pass Node
         * @note usage must be valid for the Pass queue family type
         * @note usage must not conflict with the previous usages of this texture in the whole program
         */
        void read(TextureID texture, TextureUsage usage);

        /**
         * @brief Specifies, that this Pass will read this buffer
         *
         * @param buffer Virtual buffer ID for reading
         * @param usage Type of the passed buffer usage
         *
         * @note buffer must be given you from the Render Graph to which you will push this Pass Node
         * @note usage must be valid for the Pass queue family type
         * @note usage must not conflict with the previous usages of this buffer in the whole program
         */
        void read(BufferID buffer, BufferUsage usage);

        /**
         * @brief Sets your function to write commands to the command buffer
         * @param func Function to call during the Execute Phase
         */
        void callback(std::function<void(vk::CommandBuffer)> func);

        /**
         * @brief Says whether the Pass Node described completely
         * @return True if the Pass Node described completely, false otherwise
         */
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
