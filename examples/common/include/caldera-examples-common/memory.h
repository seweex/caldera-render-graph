#ifndef CALDERA_EXAMPLES_COMMON_MEMORY_H
#define CALDERA_EXAMPLES_COMMON_MEMORY_H

#include <caldera-examples-common/config.h>
#include <vk_mem_alloc.h>

namespace caldera_examples_common
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
            vk::ImageCreateInfo const& imageInfo, VmaAllocationCreateInfo const& allocationInfo) const;

        [[nodiscard]] std::pair<vk::Buffer, VmaAllocation> create_buffer(
            vk::BufferCreateInfo const& bufferInfo, VmaAllocationCreateInfo const& allocationInfo) const;

        VmaAllocator allocator;
    };
}

#endif