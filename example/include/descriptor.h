#ifndef CALDERA_EXAMPLE_DESCRIPTOR_H
#define CALDERA_EXAMPLE_DESCRIPTOR_H

#include <vulkan_include.h>

namespace caldera_example
{
    struct Device;

    struct BindlessLayout
    {
        static constexpr uint32_t max_descriptors_count = 100;

    private:
        [[nodiscard]] static vk::DescriptorSetLayout create_descriptors_layout(vk::Device device);

        [[nodiscard]] static vk::PipelineLayout create_pipeline_layout(
            vk::Device device, vk::DescriptorSetLayout layout);

    public:
        BindlessLayout() noexcept;
        ~BindlessLayout() noexcept;

        BindlessLayout(BindlessLayout &&) = delete;
        BindlessLayout& operator=(BindlessLayout &&) = delete;

        BindlessLayout(BindlessLayout const&) = delete;
        BindlessLayout& operator=(BindlessLayout const&) = delete;

        [[nodiscard]] bool init(Device const& device);
        void clear() noexcept;

    private:
        vk::Device m_device;

    public:
        vk::DescriptorSetLayout descriptorsLayout;
        vk::PipelineLayout pipelineLayout;
    };

    struct BindlessDescriptors
    {
    private:
        [[nodiscard]] static vk::DescriptorPool create_pool(vk::Device device);

        [[nodiscard]] static vk::DescriptorSet allocate_set(
            vk::Device device, vk::DescriptorPool pool, vk::DescriptorSetLayout layout);

    public:
        BindlessDescriptors() noexcept;
        ~BindlessDescriptors() noexcept;

        BindlessDescriptors(BindlessDescriptors &&) = delete;
        BindlessDescriptors& operator=(BindlessDescriptors &&) = delete;

        BindlessDescriptors(BindlessDescriptors const&) = delete;
        BindlessDescriptors& operator=(BindlessDescriptors const&) = delete;

        [[nodiscard]] bool init(Device const& device, BindlessLayout const& layout);
        void clear() noexcept;

    private:
        vk::Device m_device;

    public:
        vk::DescriptorPool pool;
        vk::DescriptorSet set;

        uint32_t nextFreeSlot;
    };
}

#endif
