
#include <caldera-examples-common/descriptor.h>
#include <caldera-examples-common/device.h>

#include <spdlog/spdlog.h>
#include <glm/glm.hpp>

namespace caldera_examples_common
{
    /* * * * * * * * * */
    /* Bindless Layout */
    /* * * * * * * * * */

    vk::DescriptorSetLayout BindlessLayout::create_descriptors_layout(vk::Device const device)
    {
        vk::DescriptorSetLayoutBinding constexpr binding {
            0, vk::DescriptorType::eCombinedImageSampler, BindlessLayout::max_descriptors_count, vk::ShaderStageFlagBits::eFragment
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
            vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4)
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

    BindlessLayout::operator LayoutProxy() const noexcept
    {
        return LayoutProxy {
            descriptorsLayout,
            pipelineLayout
        };
    }

    /* * * * * * * * * * *  */
    /* Bindless Descriptors */
    /* * * * * * * * * * *  */

    vk::DescriptorPool BindlessDescriptors::create_pool(vk::Device const device)
    {
        vk::DescriptorPoolSize constexpr poolSize {
            vk::DescriptorType::eCombinedImageSampler, BindlessLayout::max_descriptors_count
        };

        vk::DescriptorPoolCreateInfo const createInfo
        {
            vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
            1, 1, &poolSize
        };

        auto const newPool = device.createDescriptorPool(createInfo);

        if (!newPool.has_value())
        {
            spdlog::error("Failed to create a pool: {}", vk::to_string(newPool.result));
            return VK_NULL_HANDLE;
        }

        return *newPool;
    }

    vk::DescriptorSet BindlessDescriptors::allocate_set(
        vk::Device const device,
        vk::DescriptorPool const pool,
        vk::DescriptorSetLayout const layout)
    {
        vk::StructureChain const allocateInfo
        {
            vk::DescriptorSetAllocateInfo{
                pool, 1, &layout
            },
            vk::DescriptorSetVariableDescriptorCountAllocateInfo{
                1, &BindlessLayout::max_descriptors_count
            }
        };

        auto const newSet = device.allocateDescriptorSets(allocateInfo.get<>());

        if (!newSet.has_value())
        {
            spdlog::error("Failed to allocate a descriptor set: {}", vk::to_string(newSet.result));
            return VK_NULL_HANDLE;
        }

        return newSet->front();
    }

    BindlessDescriptors::BindlessDescriptors() noexcept = default;

    BindlessDescriptors::~BindlessDescriptors() noexcept {
        clear();
    }

    bool BindlessDescriptors::init(Device const& device, BindlessLayout const& layout)
    {
        m_device = device.device;
        nextFreeSlot = 0;

        if (!(pool = create_pool(m_device)) ||
            !(set = allocate_set(m_device, pool, layout.descriptorsLayout)))
        {
            clear();
            return false;
        }

        return true;
    }

    void BindlessDescriptors::clear() noexcept
    {
        if (m_device)
        {
            m_device.destroyDescriptorPool(pool);

            pool = VK_NULL_HANDLE;
            set = VK_NULL_HANDLE;
            m_device = VK_NULL_HANDLE;
        }
    }
}
