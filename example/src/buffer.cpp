
#include <buffer.h>
#include <device.h>
#include <memory.h>

namespace caldera_example
{
    VmaAllocationCreateInfo Buffer::create_info(Settings settings) noexcept
    {
        VmaAllocationCreateInfo result{};
        result.usage = VMA_MEMORY_USAGE_AUTO;

        switch (settings.memoryType)
        {
        case MemoryType::gpu_local:
            result.flags = 0;
            break;

        case MemoryType::host_writable:
            result.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            break;

        case MemoryType::constantly_mapped:
            result.flags =
                VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                VMA_ALLOCATION_CREATE_MAPPED_BIT;

            break;
        }

        return result;
    }

    Buffer::Buffer() noexcept = default;

    Buffer::~Buffer() noexcept {
        clear();
    }

    bool Buffer::init(Device const& device, Allocator const& allocator, Settings settings)
    {
        auto const allocationInfo = create_info(settings);

        vk::BufferCreateInfo const createInfo
        {
            vk::BufferCreateFlags{},
            settings.size,
            settings.usage,
            vk::SharingMode::eExclusive,
            1, &device.queueFamilyIndex
        };

        auto const [newBuffer, newAllocation] = allocator.create_buffer(createInfo, allocationInfo);

        if (!newBuffer)
            return false;

        m_allocator = allocator.allocator;
        buffer = newBuffer;
        allocation = newAllocation;

        return true;
    }

    void Buffer::clear() noexcept
    {
        if (m_allocator)
        {
            vmaDestroyBuffer(m_allocator, buffer, allocation);

            m_allocator = VK_NULL_HANDLE;
            buffer = VK_NULL_HANDLE;
            allocation = VK_NULL_HANDLE;
        }
    }

    void* Buffer::get_constant_mapping() noexcept
    {
        VmaAllocationInfo info;
        vmaGetAllocationInfo(m_allocator, allocation, &info);

        return info.pMappedData;
    }
}
