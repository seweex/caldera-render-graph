#ifndef CALDERA_RESOURCE_H
#define CALDERA_RESOURCE_H

#include <cstdint>

namespace caldera
{
    struct TextureID { uint32_t id; };
    struct BufferID { uint32_t id; };

    enum class TextureUsage : uint8_t
    {
        color_attachment,
        depth_attachment,
        shader_usage,
        transfer,
        present
    };

    enum class BufferUsage : uint8_t
    {
        storage,
        uniform,
        vertex,
        index,
        transfer,
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
