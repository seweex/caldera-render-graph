
#include <../../common/include/image.h>
#include <memory.h>
#include <../../common/include/device.h>

#include <spdlog/spdlog.h>

namespace caldera_example
{
    bool Image::check_limits(vk::PhysicalDevice const device, Settings const settings)
    {
        auto const properties = device.getImageFormatProperties(
            settings.format, vk::ImageType::e2D, vk::ImageTiling::eOptimal, settings.usage, vk::ImageCreateFlags{});

        if (!properties.has_value())
        {
            if (properties.result == vk::Result::eErrorFormatNotSupported)
                spdlog::error("Image format is incompatible: {}", vk::to_string(settings.format));
            else
                spdlog::error("Failed to get format properties: {}", vk::to_string(properties.result));

            return false;
        }

        if (properties->maxExtent.width < settings.extent.width ||
            properties->maxExtent.height < settings.extent.height)
        {
            spdlog::error("Image extent {}x{} isn't supported", settings.extent.width, settings.extent.height);
            return false;
        }

        return true;
    }

    Image::Image() noexcept = default;

    Image::~Image() noexcept {
        clear();
    }

    bool Image::init(Device const& device, Allocator const& allocator, Settings const settings)
    {
        if (!check_limits(device.physicalDevice, settings))
            return false;

        VmaAllocationCreateInfo constexpr allocationInfo {
            VmaAllocationCreateFlags{},
            VMA_MEMORY_USAGE_AUTO
        };

        vk::ImageCreateInfo const createInfo
        {
            vk::ImageCreateFlags{},
            vk::ImageType::e2D,
            settings.format,
            vk::Extent3D{ settings.extent, 1 },
            1, 1,
            vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            settings.usage,
            vk::SharingMode::eExclusive,
            1, &device.queueFamilyIndex,
            vk::ImageLayout::eUndefined
        };

        auto const [newImage, newAllocation] = allocator.create_image(createInfo, allocationInfo);

        if (!newImage)
            return false;

        m_allocator = allocator.allocator;
        image = newImage;
        allocation = newAllocation;

        return true;
    }

    void Image::clear() noexcept
    {
        if (m_allocator)
        {
            vmaDestroyImage(m_allocator, image, allocation);

            m_allocator = VK_NULL_HANDLE;
            image = VK_NULL_HANDLE;
            allocation = VK_NULL_HANDLE;
        }
    }
}
