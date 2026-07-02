#ifndef CALDERA_EXAMPLE_SHADER_H
#define CALDERA_EXAMPLE_SHADER_H

#include <span>

#include <vulkan/vulkan.hpp>

namespace caldera_example
{
    struct Device;

    struct Shader
    {
        Shader() noexcept;
        ~Shader() noexcept;

        Shader(Shader &&) = delete;
        Shader& operator=(Shader &&) = delete;

        Shader(Shader const&) = delete;
        Shader& operator=(Shader const&) = delete;

        [[nodiscard]] bool init(Device const& device, std::span<uint32_t const> binary);
        void clear() noexcept;

    private:
        vk::Device m_device;

    public:
        vk::ShaderModule module;
    };
}

#endif
