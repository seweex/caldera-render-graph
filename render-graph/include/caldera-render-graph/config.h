#ifndef CALDERA_RENDER_GRAPH_CONFIG_H
#define CALDERA_RENDER_GRAPH_CONFIG_H

/* Frequent includes & definitions */

#define VULKAN_HPP_ASSERT_ON_RESULT(expr) ((void)(expr))
#include <vulkan/vulkan.hpp>

/* Forward declarations */

/// @brief Main namespace of 'Calder - Render Graph' project
namespace caldera
{
    struct TextureID;
    struct BufferID;

    enum class TextureUsage : uint8_t;
    enum class BufferUsage : uint8_t;

    enum class QueueType;

    class PassNode;
    class RenderGraph;

    struct QueueIndices;
    class Scheduler;
}

/// @internal
namespace caldera::detail
{
    enum class ResourceAccess : uint8_t;

    enum class TransitionRequirements;
    enum class TransitionType;

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

    struct SubmissionID;
}

#endif
