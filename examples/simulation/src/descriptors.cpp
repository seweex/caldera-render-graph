
#include <caldera-examples-common/device.h>
#include <caldera-examples-common/descriptor.h>

#include <caldera-examples-simulation/descriptors.h>

#include <spdlog/spdlog.h>

namespace
{
    [[nodiscard]] vk::DescriptorPool create_pool(vk::Device const device)
    {
        vk::DescriptorPoolSize constexpr size { vk::DescriptorType::eStorageBuffer, 1 };

        vk::DescriptorPoolCreateInfo const createInfo {
            vk::DescriptorPoolCreateFlags{},
            1, size
        };

        if (auto const newPool = device.createDescriptorPool(createInfo);
           !newPool.has_value())
        {
            spdlog::error("Failed to create a descriptor pool: {}", vk::to_string(newPool.result));
            return VK_NULL_HANDLE;
        }
        else return *newPool;
    }

    [[nodiscard]] vk::DescriptorSet create_set(
        vk::Device const device,
        vk::DescriptorPool const pool,
        vk::DescriptorSetLayout const layout)
    {
        vk::DescriptorSetAllocateInfo const allocateInfo{ pool, 1, &layout };

        if (auto const newSet = device.allocateDescriptorSets(allocateInfo);
            !newSet.has_value())
        {
            spdlog::error("Failed to allocate a descriptor set: {}", vk::to_string(newSet.result));
            return VK_NULL_HANDLE;
        }
        else return newSet->front();
    }
}

namespace caldera_examples_simulation
{
    ComputeDescriptors::ComputeDescriptors() noexcept = default;
    ComputeDescriptors::~ComputeDescriptors() noexcept {
        clear();
    }

    bool ComputeDescriptors::init(common::Device const& device, common::LayoutProxy const layoutProxy)
    {
        m_device = device.device;

        if (!(pool = create_pool(m_device)) ||
            !(set = create_set(m_device, pool, layoutProxy.descriptorsLayout)))
        {
            return false;
        }

        return true;
    }

    void ComputeDescriptors::clear() noexcept
    {
        if (m_device)
        {
            m_device.destroyDescriptorPool(pool);

            pool = VK_NULL_HANDLE;
            set = VK_NULL_HANDLE;
            m_device = VK_NULL_HANDLE;
        }
    }

    void ComputeDescriptors::bind(vk::Buffer const buffer)
    {
        vk::DescriptorBufferInfo const bufferInfo { buffer, 0, vk::WholeSize };

        vk::WriteDescriptorSet const writeInfo {
            set, 0, 0, 1, vk::DescriptorType::eStorageBuffer,
            nullptr, &bufferInfo, nullptr
        };

        m_device.updateDescriptorSets(writeInfo, {});
    }
}
