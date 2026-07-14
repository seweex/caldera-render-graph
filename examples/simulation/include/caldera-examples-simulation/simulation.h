#ifndef CALDERA_EXAMPLE_SIMULATION_SIMULATION_H
#define CALDERA_EXAMPLE_SIMULATION_SIMULATION_H

#include <glm/glm.hpp>

namespace caldera_examples_simulation
{
    inline auto constexpr planet_count = 128;
    inline auto constexpr gravity = 0.1f;

    struct Planet
    {
        glm::vec4 position_and_mass;
        glm::vec4 color_and_radius;
        glm::vec4 speed;
    };

    struct Settings
    {
        glm::mat4 cameraMatrix;
        float deltaTime;
        float timeSpeed;
    };
}

#endif
