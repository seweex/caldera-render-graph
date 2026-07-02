#ifndef CALDERA_EXAMPLE_PIPELINE_H
#define CALDERA_EXAMPLE_PIPELINE_H

#include <vulkan/vulkan.hpp>

namespace caldera_example
{
    struct Device;

    struct BindlessLayout
    {
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
}

#endif
