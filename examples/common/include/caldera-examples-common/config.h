#ifndef CALDERA_EXAMPLES_COMMON_CONFIG_H
#define CALDERA_EXAMPLES_COMMON_CONFIG_H

#define VULKAN_HPP_ASSERT_ON_RESULT(expr) ((void)(expr))
#include <vulkan/vulkan.hpp>

namespace caldera_examples_common
{
    struct Context;
    struct Window;
    struct Device;

    struct Swapchain;
    struct FrameResources;
    struct Scheduler;

    struct Allocator;
    struct Image;
    struct Buffer;
    struct Shader;

    struct LayoutProxy;
    struct BindlessLayout;
    struct BindlessDescriptors;

    struct Renderer;
}

#endif
