
#include <caldera-examples-common/device.h>
#include <caldera-examples-common/shader.h>
#include <caldera-examples-common/descriptor.h>
#include <caldera-examples-common/swapchain.h>
#include <caldera-examples-common/window.h>

#include <caldera-examples-simulation/pipeline.h>
#include <caldera-examples-simulation/simulation.h>

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace
{
    [[nodiscard]] vk::DescriptorSetLayout create_descriptors_layout(vk::Device const device)
    {
        vk::DescriptorSetLayoutBinding constexpr binding0 {
            0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute
        };

        vk::DescriptorSetLayoutCreateInfo const createInfo {
            vk::DescriptorSetLayoutCreateFlags{}, binding0
        };

        if (auto const newLayout = device.createDescriptorSetLayout(createInfo);
           !newLayout.has_value())
        {
            spdlog::error("Failed to create a descriptor set layout: {}", vk::to_string(newLayout.result));
            return VK_NULL_HANDLE;
        }
        else return *newLayout;
    }

    [[nodiscard]] vk::PipelineLayout create_pipeline_layout(
        vk::Device const device,
        vk::DescriptorSetLayout const layout = VK_NULL_HANDLE)
    {
        vk::PushConstantRange constexpr pushConstantsInfo {
            vk::ShaderStageFlagBits::eCompute |
            vk::ShaderStageFlagBits::eVertex |
            vk::ShaderStageFlagBits::eFragment,
            0, sizeof (caldera_examples_simulation::Settings)
        };

        vk::PipelineLayoutCreateInfo const createInfo {
            vk::PipelineLayoutCreateFlags{},
            layout == VK_NULL_HANDLE ? 0u : 1u, &layout,
            1, &pushConstantsInfo,
        };

        if (auto const newLayout = device.createPipelineLayout(createInfo);
           !newLayout.has_value())
        {
            spdlog::error("Failed to create a compute pipeline layout: {}", vk::to_string(newLayout.result));
            return VK_NULL_HANDLE;
        }
        else return *newLayout;
    }

    /* Pipeline States */

    [[nodiscard]] constexpr std::array<vk::VertexInputBindingDescription, 1>
    create_vertex_input_bindings() noexcept
    {
        return { vk::VertexInputBindingDescription {
            0, sizeof(caldera_examples_simulation::Planet), vk::VertexInputRate::eVertex
        } };
    }

    [[nodiscard]] constexpr std::array<vk::VertexInputAttributeDescription, 3>
    create_vertex_input_attributes() noexcept
    {
        return {
            vk::VertexInputAttributeDescription{
                0, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(caldera_examples_simulation::Planet, position_and_mass)
            },
            vk::VertexInputAttributeDescription{
                1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(caldera_examples_simulation::Planet, color_and_radius)
            },
            vk::VertexInputAttributeDescription{
                2, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(caldera_examples_simulation::Planet, speed)
            }
        };
    }

    [[nodiscard]] constexpr std::array<vk::PipelineColorBlendAttachmentState, 1>
    create_blend_attachment() noexcept
    {
        vk::PipelineColorBlendAttachmentState attachment_0 = {};
        attachment_0.blendEnable = vk::False;
        attachment_0.colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

        return {
            attachment_0
        };
    }

    [[nodiscard]] std::pair<vk::Viewport, vk::Rect2D>
    get_viewport_and_scissors(GLFWwindow* const window) noexcept
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        return {
            vk::Viewport{ 0.f, 0.f, static_cast<float>(width), static_cast<float>(height), 0.f, 1.f },
            vk::Rect2D{ vk::Offset2D{ 0, 0 }, vk::Extent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) } }
        };
    }

    [[nodiscard]] constexpr vk::PipelineRenderingCreateInfo
    create_rendering_info() noexcept
    {
        return {
            0,
            1, &caldera_examples_common::Swapchain::target_image_format,
            vk::Format::eD32Sfloat,
            vk::Format::eUndefined
        };
    }

    /* * * * */

    [[nodiscard]] std::array<vk::PipelineShaderStageCreateInfo, 2>
    create_shader_stage(
        vk::ShaderModule const vertex,
        vk::ShaderModule const fragment) noexcept
    {
        return {
            vk::PipelineShaderStageCreateInfo
            {
                vk::PipelineShaderStageCreateFlags{},
                vk::ShaderStageFlagBits::eVertex,
                vertex,
                "main",
                nullptr
            },
            vk::PipelineShaderStageCreateInfo
            {
                vk::PipelineShaderStageCreateFlags{},
                vk::ShaderStageFlagBits::eFragment,
                fragment,
                "main",
                nullptr
            }
        };
    }

    [[nodiscard]] vk::PipelineVertexInputStateCreateInfo
    create_vertex_input_state(
        std::span<vk::VertexInputBindingDescription const> const bindings,
        std::span<vk::VertexInputAttributeDescription const> const attributes) noexcept
    {
        return {
            vk::PipelineVertexInputStateCreateFlags{},
            bindings,
            attributes
        };
    }

    [[nodiscard]] constexpr vk::PipelineInputAssemblyStateCreateInfo
    create_input_assembly_state() noexcept
    {
        return {
            vk::PipelineInputAssemblyStateCreateFlags{},
            vk::PrimitiveTopology::ePointList,
            vk::False
        };
    }

    [[nodiscard]] vk::PipelineViewportStateCreateInfo
    create_viewport_state(vk::Viewport const& viewport, vk::Rect2D const& scissors) noexcept
    {
        return {
            vk::PipelineViewportStateCreateFlags{},
            viewport, scissors
        };
    }

    [[nodiscard]] constexpr vk::PipelineRasterizationStateCreateInfo
    create_rasterization_state() noexcept
    {
        return {
            vk::PipelineRasterizationStateCreateFlags{},
            vk::False,
            vk::False,
            vk::PolygonMode::eFill,
            vk::CullModeFlagBits::eNone,
            vk::FrontFace::eClockwise,
            vk::False,
            0.f,
            0.f,
            0.f,
            1.f
        };
    }

    [[nodiscard]] constexpr vk::PipelineMultisampleStateCreateInfo
    create_multisample_state() noexcept
    {
        return {
            vk::PipelineMultisampleStateCreateFlags{},
            vk::SampleCountFlagBits::e1,
            vk::False,
            0.f,
            nullptr,
            vk::False,
            vk::False
        };
    }

    [[nodiscard]] constexpr vk::PipelineDepthStencilStateCreateInfo
    create_depth_stencil_state() noexcept
    {
        return vk::PipelineDepthStencilStateCreateInfo
        {
            vk::PipelineDepthStencilStateCreateFlags{},
            vk::True,
            vk::True,
            vk::CompareOp::eLess,
            vk::False,
            vk::False,
            vk::StencilOp::eZero,
            vk::StencilOp::eZero,
            0.f,
            1.f
        };
    }

    [[nodiscard]] vk::PipelineColorBlendStateCreateInfo
    create_blend_state(std::span<vk::PipelineColorBlendAttachmentState const> const attachments) noexcept
    {
        return {
            vk::PipelineColorBlendStateCreateFlags{},
            vk::False,
            vk::LogicOp::eClear,
            attachments,
            { 0.f, 0.f, 0.f, 0.f }
        };
    }
}

namespace caldera_examples_simulation
{
    /* * * * * * * * * */
    /* Settings Layout */
    /* * * * * * * * * */

    SettingsLayout::SettingsLayout() noexcept = default;
    SettingsLayout::~SettingsLayout() noexcept {
        clear();
    }

    bool SettingsLayout::init(common::Device const& device)
    {
        m_device = device.device;

        if (!(pipelineLayout = create_pipeline_layout(m_device)))
        {
            clear();
            return false;
        }

        return true;
    }

    void SettingsLayout::clear() noexcept
    {
        if (m_device)
        {
            m_device.destroyPipelineLayout(pipelineLayout);

            pipelineLayout = VK_NULL_HANDLE;
            m_device = VK_NULL_HANDLE;
        }
    }

    SettingsLayout::operator common::LayoutProxy() const noexcept {
        return common::LayoutProxy{ VK_NULL_HANDLE, pipelineLayout };
    }

    /* * * * ** * * * */
    /* Compute Layout */
    /* * * * ** * * * */

    ComputeLayout::ComputeLayout() noexcept = default;
    ComputeLayout::~ComputeLayout() noexcept {
        clear();
    }

    bool ComputeLayout::init(common::Device const& device)
    {
        m_device = device.device;

        if (!(descriptorLayout = create_descriptors_layout(m_device)) ||
            !(pipelineLayout = create_pipeline_layout(m_device, descriptorLayout)))
        {
            clear();
            return false;
        }

        return true;
    }

    void ComputeLayout::clear() noexcept
    {
        if (m_device)
        {
            m_device.destroyPipelineLayout(pipelineLayout);
            m_device.destroyDescriptorSetLayout(descriptorLayout);

            descriptorLayout = VK_NULL_HANDLE;
            pipelineLayout = VK_NULL_HANDLE;
            m_device = VK_NULL_HANDLE;
        }
    }

    ComputeLayout::operator caldera_examples_common::LayoutProxy() const noexcept {
        return common::LayoutProxy{ descriptorLayout, pipelineLayout };
    }

    /* * * * ** * * * * */
    /* Compute Pipeline */
    /* * * * ** * * * * */

    ComputePipeline::ComputePipeline() noexcept = default;
    ComputePipeline::~ComputePipeline() noexcept {
        clear();
    }

    bool ComputePipeline::init(
        common::Device const& device,
        common::LayoutProxy const& layout,
        common::Shader const& shader)
    {
        vk::PipelineShaderStageCreateInfo const shaderStageCreateInfo {
            vk::PipelineShaderStageCreateFlags{},
            vk::ShaderStageFlagBits::eCompute,
            shader.module,
            "main"
        };

        vk::ComputePipelineCreateInfo const createInfo {
            vk::PipelineCreateFlags{},
            shaderStageCreateInfo,
            layout.pipelineLayout
        };

        m_device = device.device;

        if (auto const newLayout = m_device.createComputePipeline(VK_NULL_HANDLE, createInfo);
            !newLayout.has_value())
        {
            spdlog::error("Failed to create a compute pipeline: {}", vk::to_string(newLayout.result));
            clear();
            return false;
        }
        else pipeline = *newLayout;

        return true;
    }

    void ComputePipeline::clear() noexcept
    {
        if (m_device)
        {
            m_device.destroyPipeline(pipeline);

            pipeline = VK_NULL_HANDLE;
            m_device = VK_NULL_HANDLE;
        }
    }

    /* * * * * * * * * * */
    /* Graphics Pipeline */
    /* * * * * * * * * * */

    GraphicsPipeline::GraphicsPipeline() noexcept = default;
    GraphicsPipeline::~GraphicsPipeline() noexcept {
        clear();
    }

    bool GraphicsPipeline::init(
        common::Window const& window,
        common::Device const& device,
        common::LayoutProxy const& layout,
        common::Shader const& vertex,
        common::Shader const& fragment)
    {
        auto constexpr vertex_input_bindings = create_vertex_input_bindings();
        auto constexpr vertex_input_attributes = create_vertex_input_attributes();
        auto constexpr blend_attachments = create_blend_attachment();

        auto const shaderStages = create_shader_stage(vertex.module, fragment.module);
        auto const vertexInputState = create_vertex_input_state(vertex_input_bindings, vertex_input_attributes);
        auto const blendState = create_blend_state(blend_attachments);

        auto const [viewport, scissors] = get_viewport_and_scissors(window.window.get());
        auto const viewportState = create_viewport_state(viewport, scissors);

        auto constexpr input_assembly_state = create_input_assembly_state();
        auto constexpr rasterization_state = create_rasterization_state();
        auto constexpr multisample_state = create_multisample_state();
        auto constexpr depth_stencil_state = create_depth_stencil_state();
        auto constexpr rendering_info = create_rendering_info();

        vk::StructureChain const createInfo
        {
            vk::GraphicsPipelineCreateInfo
            {
                vk::PipelineCreateFlags{},
                shaderStages,
                &vertexInputState,
                &input_assembly_state,
                nullptr,
                &viewportState,
                &rasterization_state,
                &multisample_state,
                &depth_stencil_state,
                &blendState,
                nullptr,
                layout.pipelineLayout,
                VK_NULL_HANDLE, 0
            },
            rendering_info
        };

        auto const newPipeline = device.device.createGraphicsPipeline(VK_NULL_HANDLE, createInfo.get<>());

        if (!newPipeline.has_value())
        {
            spdlog::error("Failed to create pipeline: {}", vk::to_string(newPipeline.result));
            return false;
        }

        m_device = device.device;
        pipeline = newPipeline.value;

        return true;
    }

    void GraphicsPipeline::clear() noexcept
    {
        if (m_device)
        {
            m_device.destroyPipeline(pipeline);

            pipeline = VK_NULL_HANDLE;
            m_device = VK_NULL_HANDLE;
        }
    }
}
