#ifndef CALDERA_FWD_H
#define CALDERA_FWD_H

/* Frequent includes & definitions */

#define VULKAN_HPP_ASSERT_ON_RESULT(expr) ((void)(expr))
#include <vulkan/vulkan.hpp>

/* Forward declarations */

namespace caldera
{
    struct TextureID;
    struct BufferID;

    enum class TextureUsage : uint8_t;
    enum class BufferUsage : uint8_t;

    enum class QueueType;

    class PassNode;
    class RenderGraph;
}

namespace caldera::detail
{
    enum class ResourceAccess : uint8_t;

    struct TextureState;
    struct BufferState;

    struct TextureVulkanState;
    struct BufferVulkanState;

    struct TextureBinding;
    struct BufferBinding;

    struct TextureDescription;
    struct BufferDescription;

    struct TextureTransition;
    struct BufferTransition;
}

#endif
