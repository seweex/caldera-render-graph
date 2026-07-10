
#include <caldera-examples-common/shader.h>
#include <caldera-examples-common/device.h>

#include <spdlog/spdlog.h>

namespace caldera_examples_common
{
    Shader::Shader() noexcept = default;

    Shader::~Shader() noexcept {
        clear();
    }

    bool Shader::init(Device const& device, std::span<uint32_t const> const binary)
    {
        vk::ShaderModuleCreateInfo const createInfo
        {
            vk::ShaderModuleCreateFlags{},
            binary.size() * sizeof(uint32_t),
            binary.data()
        };

        auto const newModule = device.device.createShaderModule(createInfo);

        if (!newModule.has_value())
        {
            spdlog::error("Failed to create a shader module: ", vk::to_string(newModule.result));
            return false;
        }

        m_device = device.device;
        module = *newModule;

        return true;
    }

    void Shader::clear() noexcept
    {
        if (m_device)
        {
            m_device.destroyShaderModule(module);

            module = VK_NULL_HANDLE;
            m_device = VK_NULL_HANDLE;
        }
    }
}


