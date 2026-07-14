#ifndef CALDERA_EXAMPLE_SIMULATION_DESCRIPTORS_H
#define CALDERA_EXAMPLE_SIMULATION_DESCRIPTORS_H

#include <caldera-examples-common/config.h>

namespace caldera_examples_simulation
{
    namespace common = caldera_examples_common;

    struct ComputeDescriptors
    {
        ComputeDescriptors() noexcept;
        ~ComputeDescriptors() noexcept;

        ComputeDescriptors(ComputeDescriptors &&) = delete;
        ComputeDescriptors& operator=(ComputeDescriptors &&) = delete;

        ComputeDescriptors(ComputeDescriptors const&) = delete;
        ComputeDescriptors& operator=(ComputeDescriptors const&) = delete;

        [[nodiscard]] bool init(common::Device const& device, common::LayoutProxy layoutProxy);
        void clear() noexcept;

        void bind(vk::Buffer buffer);

    private:
        vk::Device m_device;

    public:
        vk::DescriptorPool pool;
        vk::DescriptorSet set;
    };
}

#endif
