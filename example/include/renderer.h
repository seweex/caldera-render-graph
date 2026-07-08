#ifndef CALDERA_EXAMPLE_RENDERER_H
#define CALDERA_EXAMPLE_RENDERER_H

#include <vulkan_include.h>
#include <glm/fwd.hpp>

namespace caldera_example
{
    struct Renderer
    {
        void begin();
        void end();

        void bind_mesh(vk::Buffer vertices, vk::Buffer indices);
        void bind_material(vk::Pipeline pipeline);
        void push_constant(vk::PipelineLayout layout, glm::mat4 const& matrix);

        void draw();

        vk::Image image;
        vk::ImageView view;

        vk::CommandBuffer commandBuffer;
    };
}

#endif
