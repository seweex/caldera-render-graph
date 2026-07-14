#ifndef CALDERA_EXAMPLES_COMMON_IMAGE_H
#define CALDERA_EXAMPLES_COMMON_IMAGE_H

#include <caldera-examples-common/config.h>
#include <vk_mem_alloc.h>

namespace caldera_examples_common
{
    struct Device;
    struct Allocator;

    struct Image
    {
        struct Settings
        {
            vk::Format format;
            vk::Extent2D extent;
            vk::ImageUsageFlags usage;
        };

    private:
        [[nodiscard]] static bool check_limits(vk::PhysicalDevice device, Settings const settings);

    public:
        Image() noexcept;
        ~Image() noexcept;

        Image(Image &&) = delete;
        Image& operator=(Image &&) = delete;

        Image(Image const&) = delete;
        Image& operator=(Image const&) = delete;

        [[nodiscard]] bool init(Device const& device, Allocator const& allocator, Settings settings);
        void clear() noexcept;

    private:
        VmaAllocator m_allocator;

    public:
        vk::Image image;
        VmaAllocation allocation;
    };
}

#endif
