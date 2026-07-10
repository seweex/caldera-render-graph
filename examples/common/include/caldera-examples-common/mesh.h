#ifndef CALDERA_EXAMPLES_COMMON_MESH_H
#define CALDERA_EXAMPLES_COMMON_MESH_H

#include <array>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace caldera_examples_common
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 uv;
        glm::vec3 normal;
    };

    inline constexpr std::array cube_vertices
    {
        Vertex{ glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f,  1.0f) },
        Vertex{ glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f,  1.0f) },
        Vertex{ glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f,  1.0f) },
        Vertex{ glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f,  1.0f) },

        Vertex{ glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f) },
        Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f) },
        Vertex{ glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f) },
        Vertex{ glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f) },

        Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(-1.0f, 0.0f,  0.0f) },
        Vertex{ glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 1.0f), glm::vec3(-1.0f, 0.0f,  0.0f) },
        Vertex{ glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f) },
        Vertex{ glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f) },

        Vertex{ glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 1.0f), glm::vec3( 1.0f, 0.0f,  0.0f) },
        Vertex{ glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f), glm::vec3( 1.0f, 0.0f,  0.0f) },
        Vertex{ glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 0.0f), glm::vec3( 1.0f, 0.0f,  0.0f) },
        Vertex{ glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 0.0f), glm::vec3( 1.0f, 0.0f,  0.0f) },

        Vertex{ glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f,  1.0f, 0.0f) },
        Vertex{ glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f,  1.0f, 0.0f) },
        Vertex{ glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f,  1.0f, 0.0f) },
        Vertex{ glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f,  1.0f, 0.0f) },

        Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
        Vertex{ glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
        Vertex{ glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
        Vertex{ glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) }
    };

    inline constexpr std::array<uint32_t, 36> cube_indices
    {
        0,  1,  2,  2,  3,  0,
        4,  5,  6,  6,  7,  4,
        8,  9,  10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };
}

#endif
