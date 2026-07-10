
#include <caldera-examples-common/memory.h>
#include <caldera-examples-common/device.h>
#include <caldera-examples-common/window.h>

#include <spdlog/spdlog.h>

namespace caldera_examples_common
{
    Allocator::Allocator() noexcept = default;

    Allocator::~Allocator() noexcept {
        clear();
    }

    std::pair<vk::Image, VmaAllocation> Allocator::create_image(
        vk::ImageCreateInfo const& imageInfo,
        VmaAllocationCreateInfo const& allocationInfo) const
    {
        VkImage image;
        VmaAllocation allocation;

        VkImageCreateInfo const vkImageInfo = imageInfo;

        if (auto const result = vmaCreateImage(allocator, &vkImageInfo, &allocationInfo, &image, &allocation, nullptr);
            result < VK_SUCCESS)
        {
            spdlog::error("Failed to create an image: {}", vk::to_string(static_cast<vk::Result>(result)));
            return { VK_NULL_HANDLE, VK_NULL_HANDLE };
        }

        return { image, allocation };
    }

    std::pair<vk::Buffer, VmaAllocation> Allocator::create_buffer(
        vk::BufferCreateInfo const& bufferInfo,
        VmaAllocationCreateInfo const& allocationInfo) const
    {
        VkBuffer buffer;
        VmaAllocation allocation;

        VkBufferCreateInfo const vkBufferInfo = bufferInfo;

        if (auto const result = vmaCreateBuffer(allocator, &vkBufferInfo, &allocationInfo, &buffer, &allocation, nullptr);
            result < VK_SUCCESS)
        {
            spdlog::error("Failed to create a buffer: {}", vk::to_string(static_cast<vk::Result>(result)));
            return { VK_NULL_HANDLE, VK_NULL_HANDLE };
        }

        return { buffer, allocation };
    }

    bool Allocator::init(Context const& context, Device const& device)
    {
        VmaAllocatorCreateInfo const createInfo
        {
            VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
            device.physicalDevice,
            device.device,
            0,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            context.instance,
            context.version,
            nullptr
        };

        if (auto const result = vmaCreateAllocator(&createInfo, &allocator);
            result < VK_SUCCESS)
        {
            spdlog::error("Failed to create an allocator: {}",
                vk::to_string(static_cast<vk::Result>(result)));

            return false;
        }

        return true;
    }

    void Allocator::clear() noexcept
    {
        if (allocator)
        {
            vmaDestroyAllocator(allocator);
            allocator = VK_NULL_HANDLE;
        }
    }
}
