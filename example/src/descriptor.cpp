
#include <descriptor.h>

#include <device.h>
#include <spdlog/spdlog.h>

namespace caldera_example
{
    vk::DescriptorSetLayout BindlessLayout::create_descriptors_layout(vk::Device const device)
    {
        vk::DescriptorSetLayoutBinding constexpr binding {
            0, vk::DescriptorType::eCombinedImageSampler, 100, vk::ShaderStageFlagBits::eFragment
        };

        auto constexpr bindingFlags =
            vk::DescriptorBindingFlagBits::ePartiallyBound |
            vk::DescriptorBindingFlagBits::eUpdateAfterBind |
            vk::DescriptorBindingFlagBits::eVariableDescriptorCount;

        vk::StructureChain const createInfo
        {
            vk::DescriptorSetLayoutCreateInfo {
                vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
                1, &binding
            },
            vk::DescriptorSetLayoutBindingFlagsCreateInfo {
                1, &bindingFlags
            }
        };

        auto const newLayout = device.createDescriptorSetLayout(createInfo.get<>());

        if (!newLayout.has_value())
        {
            spdlog::error("Failed to create a descriptor set layout: ", vk::to_string(newLayout.result));
            return VK_NULL_HANDLE;
        }

        return *newLayout;
    }

    vk::PipelineLayout BindlessLayout::create_pipeline_layout(
        vk::Device const device,
        vk::DescriptorSetLayout const layout)
    {
        vk::PushConstantRange constexpr pushConstantsRange {
            vk::ShaderStageFlagBits::eFragment, 0, sizeof(uint32_t)
        };

        vk::PipelineLayoutCreateInfo const createInfo
        {
            vk::PipelineLayoutCreateFlags{},
            1, &layout,
            1, &pushConstantsRange
        };

        auto const newLayout = device.createPipelineLayout(createInfo);

        if (!newLayout.has_value())
        {
            spdlog::error("Failed to create a pipeline layout: ", vk::to_string(newLayout.result));
            return VK_NULL_HANDLE;
        }

        return *newLayout;
    }

    BindlessLayout::BindlessLayout() noexcept = default;

    BindlessLayout::~BindlessLayout() noexcept {
        clear();
    }

    bool BindlessLayout::init(Device const& device)
    {
        m_device = device.device;

        if (!(descriptorsLayout = create_descriptors_layout(m_device)) ||
            !(pipelineLayout = create_pipeline_layout(m_device, descriptorsLayout)))
        {
            clear();
            return false;
        }

        return true;
    }

    void BindlessLayout::clear() noexcept
    {
        if (m_device)
        {
            m_device.destroyPipelineLayout(pipelineLayout);
            m_device.destroyDescriptorSetLayout(descriptorsLayout);

            m_device = VK_NULL_HANDLE;
            pipelineLayout = VK_NULL_HANDLE;
            descriptorsLayout = VK_NULL_HANDLE;
        }
    }
}
