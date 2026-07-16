
#include <caldera-render-graph/graph.h>
#include <caldera-render-graph/resource.h>
#include <caldera-render-graph/pass.h>

namespace
{
    [[nodiscard]] caldera::detail::TextureVulkanState
    make_init_state(caldera::detail::TextureDescription const& description)
    {
        caldera::detail::TextureVulkanState result;
        result.family = description.owningFamily;
        result.access = vk::AccessFlagBits2::eNone;
        result.stages = vk::PipelineStageFlagBits2::eAllCommands;
        result.layout = vk::ImageLayout::eUndefined;

        return result;
    }

    [[nodiscard]] caldera::detail::BufferVulkanState
    make_init_state(caldera::detail::BufferDescription const& description)
    {
        caldera::detail::BufferVulkanState result;
        result.family = description.owningFamily;
        result.access = vk::AccessFlagBits2::eNone;
        result.stages = vk::PipelineStageFlagBits2::eAllCommands;

        return result;
    }

    template <class StateTy>
    [[nodiscard]] caldera::detail::TransitionRequirements needs_transition(
        StateTy const previousLowLevelState,
        StateTy const requiredLowLevelState)
    {
        using namespace caldera;

        if constexpr (std::same_as<std::remove_cvref_t<StateTy>, detail::TextureVulkanState>)
            if (previousLowLevelState.layout == vk::ImageLayout::eUndefined ||
                previousLowLevelState.layout != requiredLowLevelState.layout)
            {
                return detail::TransitionRequirements::prevent_hazard;
            }

        if (previousLowLevelState.family != requiredLowLevelState.family)
            return detail::TransitionRequirements::transit_ownership;

        auto constexpr write_mask =
            vk::AccessFlagBits2::eShaderWrite |
            vk::AccessFlagBits2::eColorAttachmentWrite |
            vk::AccessFlagBits2::eDepthStencilAttachmentWrite |
            vk::AccessFlagBits2::eTransferWrite |
            vk::AccessFlagBits2::eMemoryWrite;

        auto const previousWrites = static_cast<bool>(previousLowLevelState.access & write_mask);
        auto const requiredWrites = static_cast<bool>(requiredLowLevelState.access & write_mask);

        if (previousWrites || requiredWrites)
            return detail::TransitionRequirements::prevent_hazard;

        return detail::TransitionRequirements::not_required;
    }

    [[nodiscard]] vk::ImageSubresourceRange
    make_subresource(
        caldera::detail::TextureState const state,
        vk::ImageAspectFlags const imageAspects)
    {
        vk::ImageSubresourceRange result;
        result.baseArrayLayer = 0;
        result.baseMipLevel = 0;
        result.layerCount = vk::RemainingArrayLayers;
        result.levelCount = vk::RemainingMipLevels;

        switch (state.usage)
        {
        case caldera::TextureUsage::color_attachment: [[fallthrough]];
        case caldera::TextureUsage::transfer: [[fallthrough]];
        case caldera::TextureUsage::present:
            result.aspectMask = vk::ImageAspectFlagBits::eColor;
            break;

        case caldera::TextureUsage::depth_attachment:
            result.aspectMask =
                vk::ImageAspectFlagBits::eDepth |
                vk::ImageAspectFlagBits::eStencil;
            break;

        case caldera::TextureUsage::shader_usage:
            result.aspectMask =
                vk::ImageAspectFlagBits::eColor |
                vk::ImageAspectFlagBits::eDepth;
            break;
        }

        result.aspectMask = result.aspectMask & imageAspects;
        return result;
    }

    [[nodiscard]] caldera::detail::TextureTransition
    make_transition(
        caldera::TextureID const id,
        caldera::detail::TransitionType const type,
        caldera::detail::TextureDescription const& description,
        caldera::detail::TextureState const binding,
        caldera::detail::TextureVulkanState const& prevState,
        caldera::detail::TextureVulkanState const& reqState)
    {
        caldera::detail::TextureTransition result;
        result.id = id;
        result.subresource = make_subresource(binding, description.aspects);

        switch (type)
        {
        case caldera::detail::TransitionType::anti_hazard:
            result.srcState = prevState;
            result.dstState = reqState;
            break;

        case caldera::detail::TransitionType::ownership_acquisition:
            result.srcState.access = vk::AccessFlagBits2::eNone;
            result.srcState.stages = vk::PipelineStageFlagBits2::eNone;
            result.srcState.layout = prevState.layout;
            result.srcState.family = prevState.family;

            result.dstState = reqState;
            break;

        case caldera::detail::TransitionType::ownership_release:
            result.dstState.access = vk::AccessFlagBits2::eNone;
            result.dstState.stages = vk::PipelineStageFlagBits2::eNone;
            result.dstState.layout = reqState.layout;
            result.dstState.family = reqState.family;

            result.srcState = prevState;
            break;
        }

        return result;
    }

    [[nodiscard]] caldera::detail::BufferTransition
    make_transition(
        caldera::BufferID const id,
        caldera::detail::TransitionType const type,
        caldera::detail::BufferVulkanState const& prevState,
        caldera::detail::BufferVulkanState const& reqState)
    {
        caldera::detail::BufferTransition result;
        result.id = id;

        switch (type)
        {
        case caldera::detail::TransitionType::anti_hazard:
            result.srcState = prevState;
            result.dstState = reqState;
            break;

        case caldera::detail::TransitionType::ownership_acquisition:
            result.srcState.access = vk::AccessFlagBits2::eNone;
            result.srcState.stages = vk::PipelineStageFlagBits2::eNone;
            result.srcState.family = prevState.family;

            result.dstState = reqState;
            break;

        case caldera::detail::TransitionType::ownership_release:
            result.dstState.access = vk::AccessFlagBits2::eNone;
            result.dstState.stages = vk::PipelineStageFlagBits2::eNone;
            result.dstState.family = reqState.family;

            result.srcState = prevState;
            break;
        }

        return result;
    }
}

namespace caldera
{
    uint32_t RenderGraph::choose_family(QueueType const type) const noexcept
    {
        switch (type)
        {
        case QueueType::graphics:
            return m_graphicsFamily;

        case QueueType::transfer:
            return m_transferFamily;

        case QueueType::compute:
            return m_computeFamily;

        default:
            return vk::QueueFamilyIgnored;
        }
    }

    detail::TextureVulkanState RenderGraph::gen_low_level_state(
        detail::TextureDescription const& description,
        detail::TextureState const state) const
    {
        if (state.queue == QueueType::none)
            return make_init_state(description);

        detail::TextureVulkanState result;
        result.family = choose_family(state.queue);

        switch (state.usage)
        {
        case TextureUsage::color_attachment:
            result.access = (state.access == detail::ResourceAccess::read) ?
                vk::AccessFlagBits2::eColorAttachmentRead : vk::AccessFlagBits2::eColorAttachmentWrite;

            result.stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
            result.layout = vk::ImageLayout::eColorAttachmentOptimal;
            break;

        case TextureUsage::depth_attachment:
            result.access = (state.access == detail::ResourceAccess::read) ?
                vk::AccessFlagBits2::eDepthStencilAttachmentRead : vk::AccessFlagBits2::eDepthStencilAttachmentWrite;

            result.stages =
                vk::PipelineStageFlagBits2::eEarlyFragmentTests |
                vk::PipelineStageFlagBits2::eLateFragmentTests;

            result.layout = (state.access == detail::ResourceAccess::read) ?
                vk::ImageLayout::eDepthStencilReadOnlyOptimal : vk::ImageLayout::eDepthStencilAttachmentOptimal;

            break;

        case TextureUsage::shader_usage:
            result.access = (state.access == detail::ResourceAccess::read) ?
                vk::AccessFlagBits2::eShaderRead : vk::AccessFlagBits2::eShaderWrite;

            result.stages =
                vk::PipelineStageFlagBits2::eFragmentShader |
                vk::PipelineStageFlagBits2::eComputeShader;

            result.layout = (state.access == detail::ResourceAccess::read) ?
                vk::ImageLayout::eShaderReadOnlyOptimal : vk::ImageLayout::eDepthStencilReadOnlyOptimal;
            break;

        case TextureUsage::transfer:
            result.access = (state.access == detail::ResourceAccess::read) ?
                vk::AccessFlagBits2::eTransferRead : vk::AccessFlagBits2::eTransferWrite;

            result.stages =
                vk::PipelineStageFlagBits2::eCopy |
                vk::PipelineStageFlagBits2::eClear;

            result.layout = (state.access == detail::ResourceAccess::read) ?
                vk::ImageLayout::eTransferSrcOptimal : vk::ImageLayout::eTransferDstOptimal;
            break;

        case TextureUsage::present:
            result.access = vk::AccessFlagBits2::eNone;
            result.stages = vk::PipelineStageFlagBits2::eNone;
            result.layout = vk::ImageLayout::ePresentSrcKHR;
            break;
        }

        return result;
    }

    detail::BufferVulkanState RenderGraph::gen_low_level_state(
        detail::BufferDescription const& description,
        detail::BufferState const state) const
    {
        if (state.queue == QueueType::none)
            return make_init_state(description);

        detail::BufferVulkanState result;
        result.family = choose_family(state.queue);

        switch (state.usage)
        {
        case BufferUsage::storage:
            result.access = (state.access == detail::ResourceAccess::read) ?
                vk::AccessFlagBits2::eShaderRead : vk::AccessFlagBits2::eShaderWrite;

            result.stages =
                vk::PipelineStageFlagBits2::eFragmentShader |
                vk::PipelineStageFlagBits2::eComputeShader;
            break;

        case BufferUsage::uniform:
            result.access = vk::AccessFlagBits2::eUniformRead;
            result.stages =
                vk::PipelineStageFlagBits2::eFragmentShader |
                vk::PipelineStageFlagBits2::eComputeShader;
            break;

        case BufferUsage::vertex:
            result.access = vk::AccessFlagBits2::eVertexAttributeRead;
            result.stages = vk::PipelineStageFlagBits2::eVertexAttributeInput;
            break;

        case BufferUsage::index:
            result.access = vk::AccessFlagBits2::eIndexRead;
            result.stages = vk::PipelineStageFlagBits2::eIndexInput;
            break;

        case BufferUsage::transfer:
            result.access = (state.access == detail::ResourceAccess::read) ?
                vk::AccessFlagBits2::eTransferRead : vk::AccessFlagBits2::eTransferWrite;

            result.stages =
                vk::PipelineStageFlagBits2::eCopy |
                vk::PipelineStageFlagBits2::eClear;
            break;

        case BufferUsage::mapped_usage:
            result.access = (state.access == detail::ResourceAccess::read) ?
                vk::AccessFlagBits2::eHostRead : vk::AccessFlagBits2::eHostWrite;

            result.stages = vk::PipelineStageFlagBits2::eHost;
            break;
        }

        return result;
    }

    RenderGraph::RenderGraph(
        vk::Device const device,
        uint32_t const graphicsFamily,
        uint32_t const transferFamily,
        uint32_t const computeFamily)
    :
        m_device(device),

        m_graphicsFamily(graphicsFamily),
        m_transferFamily(transferFamily),
        m_computeFamily(computeFamily)
    {}

    TextureID RenderGraph::declare_texture(
        uint32_t const owningFamily,
        vk::ImageAspectFlags const aspects)
    {
        auto const index = static_cast<uint32_t>(m_textures.size());
        auto& desc = m_textures.emplace_back();

        desc.aspects = aspects;
        desc.owningFamily = owningFamily;

        return TextureID{ index };
    }

    BufferID RenderGraph::declare_buffer(
        uint32_t const owningFamily,
        vk::DeviceSize const size)
    {
        auto const index = static_cast<uint32_t>(m_buffers.size());
        auto& desc = m_buffers.emplace_back();

        desc.size = size;
        desc.owningFamily = owningFamily;

        return BufferID{ index };
    }

    void RenderGraph::associate(
        TextureID const id,
        vk::Image const image,
        vk::ImageView const view)
    {
        auto& desc = m_textures[id.id];

        desc.image = image;
        desc.view = view;
    }

    void RenderGraph::associate(
        BufferID const id,
        vk::Buffer const buffer)
    {
        auto& desc = m_buffers[id.id];
        desc.buffer = buffer;
    }

    void RenderGraph::push_pass(PassNode pass) {
        m_passes.emplace_back(std::move(pass));
    }

    void RenderGraph::compile()
    {
        std::vector<detail::TextureVulkanState> textureStates;
        std::vector<detail::BufferVulkanState> bufferStates;

        textureStates.reserve(m_textures.size());
        bufferStates.reserve(m_buffers.size());

        for (auto const& desc : m_textures)
            textureStates.emplace_back(make_init_state(desc));

        for (auto const& desc : m_buffers)
            bufferStates.emplace_back(make_init_state(desc));

        for (auto passIt = m_passes.begin(); passIt != m_passes.end(); ++passIt)
        {
            for (auto const [id, state] : passIt->m_textureBindings)
            {
                auto const& desc = m_textures[id.id];

                auto& prevState = textureStates[id.id];
                auto reqState = gen_low_level_state(desc, state);

                switch (needs_transition(prevState, reqState))
                {
                case detail::TransitionRequirements::not_required:
                    prevState.access |= reqState.access;
                    prevState.stages |= reqState.stages;
                    continue;

                case detail::TransitionRequirements::prevent_hazard:
                    passIt->m_textureTransitions.push_back(make_transition(
                        id, detail::TransitionType::anti_hazard, desc, state, prevState, reqState));

                    break;

                case detail::TransitionRequirements::transit_ownership:
                    if (passIt != m_passes.begin())
                        std::prev(passIt)->m_textureTransitions.push_back(make_transition(
                            id, detail::TransitionType::ownership_release, desc, state, prevState, reqState));

                    passIt->m_textureTransitions.push_back(make_transition(
                        id, detail::TransitionType::ownership_acquisition, desc, state, prevState, reqState));

                    break;
                }

                prevState = reqState;
            }

            for (auto const [id, state] : passIt->m_bufferBindings)
            {
                auto const& desc = m_buffers[id.id];

                auto& prevState = bufferStates[id.id];
                auto reqState = gen_low_level_state(desc, state);

                switch (needs_transition(prevState, reqState))
                {
                case detail::TransitionRequirements::not_required:
                    prevState.access |= reqState.access;
                    prevState.stages |= reqState.stages;
                    continue;

                case detail::TransitionRequirements::prevent_hazard:
                    passIt->m_bufferTransitions.push_back(make_transition(
                        id, detail::TransitionType::anti_hazard, prevState, reqState));

                    break;

                case detail::TransitionRequirements::transit_ownership:
                    if (passIt != m_passes.begin())
                        std::prev(passIt)->m_bufferTransitions.push_back(make_transition(
                            id, detail::TransitionType::ownership_release, prevState, reqState));

                    passIt->m_bufferTransitions.push_back(make_transition(
                        id, detail::TransitionType::ownership_acquisition, prevState, reqState));

                    break;
                }

                prevState = reqState;
            }
        }
    }

    void RenderGraph::execute(vk::CommandBuffer const cmd)
    {
        for (auto& pass : m_passes)
        {
            pass.m_imageBarriers.clear();
            pass.m_bufferBarriers.clear();

            for (auto const& transition : pass.m_textureTransitions)
                pass.m_imageBarriers.emplace_back(
                    transition.srcState.stages,
                    transition.srcState.access,
                    transition.dstState.stages,
                    transition.dstState.access,
                    transition.srcState.layout,
                    transition.dstState.layout,
                    transition.srcState.family,
                    transition.dstState.family,
                    m_textures[transition.id.id].image,
                    transition.subresource
                );

            for (auto const& transition : pass.m_bufferTransitions)
                pass.m_bufferBarriers.emplace_back(
                    transition.srcState.stages,
                    transition.srcState.access,
                    transition.dstState.stages,
                    transition.dstState.access,
                    transition.srcState.family,
                    transition.dstState.family,
                    m_buffers[transition.id.id].buffer,
                    0,
                    m_buffers[transition.id.id].size
                );

            vk::DependencyInfo const dependencyInfo
            {
                vk::DependencyFlags{},
                {},
                pass.m_bufferBarriers,
                pass.m_imageBarriers
            };

            cmd.pipelineBarrier2(dependencyInfo);
            std::invoke(pass.m_callback, cmd);
        }
    }
}
