#ifndef CALDERA_EXAMPLE_BUFFER_H
#define CALDERA_EXAMPLE_BUFFER_H

#include <vulkan_include.h>
#include <vk_mem_alloc.h>

namespace caldera_example
{
    struct Device;
    struct Allocator;

    struct Buffer
    {
        enum class MemoryType
        {
            gpu_local,
            host_writable,
            constantly_mapped
        };

        struct Settings
        {
            uint64_t size;
            MemoryType memoryType;
            vk::BufferUsageFlags usage;
        };

    private:
        [[nodiscard]] static VmaAllocationCreateInfo create_info(Settings settings) noexcept;

    public:
        Buffer() noexcept;
        ~Buffer() noexcept;

        Buffer(Buffer &&) = delete;
        Buffer& operator=(Buffer &&) = delete;

        Buffer(Buffer const&) = delete;
        Buffer& operator=(Buffer const&) = delete;

        [[nodiscard]] bool init(Device const& device, Allocator const& allocator, Settings settings);
        void clear() noexcept;

        [[nodiscard]] void* get_constant_mapping() noexcept;

    private:
        VmaAllocator m_allocator;

    public:
        vk::Buffer buffer;
        VmaAllocation allocation;
    };
}

#endif
