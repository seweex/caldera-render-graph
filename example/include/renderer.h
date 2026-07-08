#ifndef CALDERA_EXAMPLE_RENDERER_H
#define CALDERA_EXAMPLE_RENDERER_H

#include <vulkan_include.h>

namespace caldera_example
{
    struct Renderer
    {
        void begin();
        void end();

        void bind_mesh(vk::Buffer vertices, vk::Buffer indices);
        void bind_material(vk::Pipeline pipeline);

        void draw();

        vk::Image image;
        vk::ImageView view;

        vk::CommandBuffer commandBuffer;
    };
}

#endif
