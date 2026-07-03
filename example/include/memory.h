#ifndef CALDERA_EXAMPLE_MEMORY_H
#define CALDERA_EXAMPLE_MEMORY_H

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace caldera_example
{
    struct Context;
    struct Device;

    struct Allocator
    {
        Allocator() noexcept;
        ~Allocator() noexcept;

        Allocator(Allocator &&) = delete;
        Allocator& operator=(Allocator &&) = delete;

        Allocator(Allocator const&) = delete;
        Allocator& operator=(Allocator const&) = delete;

        [[nodiscard]] bool init(Context const& context, Device const& device);
        void clear() noexcept;

        [[nodiscard]] std::pair<vk::Image, VmaAllocation> create_image(
            vk::ImageCreateInfo const& imageInfo, VmaAllocationCreateInfo const& allocationInfo);

        [[nodiscard]] std::pair<vk::Buffer, VmaAllocation> create_buffer(
            vk::BufferCreateInfo const& bufferInfo, VmaAllocationCreateInfo const& allocationInfo);

        VmaAllocator allocator;
    };
}

#endif