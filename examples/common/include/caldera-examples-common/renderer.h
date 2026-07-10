#ifndef CALDERA_EXAMPLES_COMMON_RENDERER_H
#define CALDERA_EXAMPLES_COMMON_RENDERER_H

#include <caldera-examples-common/config.h>
#include <glm/fwd.hpp>

namespace caldera_examples_common
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
