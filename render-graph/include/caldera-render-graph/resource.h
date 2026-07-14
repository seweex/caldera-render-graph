#ifndef CALDERA_RENDER_GRAPH_RESOURCE_H
#define CALDERA_RENDER_GRAPH_RESOURCE_H

#include <cstdint>

namespace caldera
{
    /// @brief Virtual texture ID
    struct TextureID { uint32_t id; };

    /// @brief Virtual buffer ID
    struct BufferID { uint32_t id; };

    /// @brief Type of texture usage in the Passes
    enum class TextureUsage : uint8_t
    {
        /**
         * @brief Specifies a texture as a color attachment
         * Used in graphic passes
         */
        color_attachment,

        /**
         * @brief Specifies a texture as a depth attachment
         * Used in graphic passes
         */
        depth_attachment,

        /**
         * @brief Specifies usage in a shaders
         * Used in graphic and compute passes
         */
        shader_usage,

        /**
         * @brief Specifies a texture as a transfer source or destination
         * Used in transfer passes
         */
        transfer,

        /**
         * @brief Specifies a texture as a present source
         * Used in graphic passes
         */
        present
    };

    /// @brief Type of buffer usage in the Passes
    enum class BufferUsage : uint8_t
    {
        /**
         * @brief Specifies a buffer as a storage
         * Used in compute and transfer passes
         */
        storage,

        /**
         * @brief Specifies a uniform buffer
         * Used in compute and graphic passes
         */
        uniform,

        /**
         * @brief Specifies a buffer as a vertex input
         * Used in graphics passes
         */
        vertex,

        /**
         * @brief Specifies a buffer as an index input
         * Used in graphics passes
         */
        index,

        /**
         * @brief Specifies a buffer as a transfer source or destination
         * Used in transfer passes
         */
        transfer,

        /**
         * @brief Specifies a buffer as a mapped copy source or destination
         * Used in transfer passes
         */
        mapped_usage
    };
}

namespace caldera::detail
{
    enum class ResourceAccess : uint8_t
    {
        write,
        read
    };

    /* Textures */

    struct TextureState
    {
        QueueType queue;
        TextureUsage usage;
        ResourceAccess access;
    };

    struct TextureVulkanState
    {
        uint32_t family;

        vk::AccessFlags2 access;
        vk::PipelineStageFlags2 stages;
        vk::ImageLayout layout;

        [[nodiscard]] bool operator==(TextureVulkanState const&) const noexcept = default;
    };

    struct TextureBinding
    {
        TextureID id;
        TextureState state;
    };

    struct TextureDescription
    {
        vk::Image image;
        vk::ImageView view;

        vk::ImageAspectFlags aspects;
    };

    struct TextureTransition
    {
        TextureID id;

        TextureVulkanState srcState;
        TextureVulkanState dstState;

        vk::ImageSubresourceRange subresource;
    };

    /* Buffers */

    struct BufferState
    {
        QueueType queue;
        BufferUsage usage;
        ResourceAccess access;
    };

    struct BufferVulkanState
    {
        uint32_t family;

        vk::AccessFlags2 access;
        vk::PipelineStageFlags2 stages;

        [[nodiscard]] bool operator==(BufferVulkanState const&) const noexcept = default;
    };

    struct BufferBinding
    {
        BufferID id;
        BufferState state;
    };

    struct BufferDescription
    {
        vk::Buffer buffer;
        vk::DeviceSize size;
    };

    struct BufferTransition
    {
        BufferID id;

        BufferVulkanState srcState;
        BufferVulkanState dstState;
    };
}

#endif
