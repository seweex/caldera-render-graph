#ifndef CALDERA_EXAMPLES_SIMULATION_PIPELINE_H
#define CALDERA_EXAMPLES_SIMULATION_PIPELINE_H

#include <caldera-examples-common/config.h>

namespace caldera_examples_simulation
{
    namespace common = caldera_examples_common;

    struct SettingsLayout
    {
        SettingsLayout() noexcept;
        ~SettingsLayout() noexcept;

        SettingsLayout(SettingsLayout &&) = delete;
        SettingsLayout& operator=(SettingsLayout &&) = delete;

        SettingsLayout(SettingsLayout const&) = delete;
        SettingsLayout& operator=(SettingsLayout const&) = delete;

        [[nodiscard]] bool init(common::Device const& device);
        void clear() noexcept;

        [[nodiscard]] operator common::LayoutProxy() const noexcept;

    private:
        vk::Device m_device;

    public:
        vk::PipelineLayout pipelineLayout;
    };

    struct ComputeLayout
    {
        ComputeLayout() noexcept;
        ~ComputeLayout() noexcept;

        ComputeLayout(ComputeLayout &&) = delete;
        ComputeLayout& operator=(ComputeLayout &&) = delete;

        ComputeLayout(ComputeLayout const&) = delete;
        ComputeLayout& operator=(ComputeLayout const&) = delete;

        [[nodiscard]] bool init(common::Device const& device);
        void clear() noexcept;

        [[nodiscard]] operator common::LayoutProxy() const noexcept;

    private:
        vk::Device m_device;

    public:
        vk::DescriptorSetLayout descriptorLayout;
        vk::PipelineLayout pipelineLayout;
    };

    struct ComputePipeline
    {
        ComputePipeline() noexcept;
        ~ComputePipeline() noexcept;

        ComputePipeline(ComputePipeline &&) = delete;
        ComputePipeline& operator=(ComputePipeline &&) = delete;

        ComputePipeline(ComputePipeline const&) = delete;
        ComputePipeline& operator=(ComputePipeline const&) = delete;

        [[nodiscard]] bool init(
            common::Device const& device,
            common::LayoutProxy const& layout,
            common::Shader const& shader);

        void clear() noexcept;

    private:
        vk::Device m_device;

    public:
        vk::Pipeline pipeline;
    };

    struct GraphicsPipeline
    {
        GraphicsPipeline() noexcept;
        ~GraphicsPipeline() noexcept;

        GraphicsPipeline(GraphicsPipeline &&) = delete;
        GraphicsPipeline& operator=(GraphicsPipeline &&) = delete;

        GraphicsPipeline(GraphicsPipeline const&) = delete;
        GraphicsPipeline& operator=(GraphicsPipeline const&) = delete;

        [[nodiscard]] bool init(
            common::Window const& window,
            common::Device const& device,
            common::LayoutProxy const& layout,
            common::Shader const& vertex,
            common::Shader const& fragment);

        void clear() noexcept;

    private:
        vk::Device m_device;

    public:
        vk::Pipeline pipeline;
    };
}

#endif
