
#include <graph.h>
#include <resource.h>
#include <pass.h>

namespace
{
    inline constexpr caldera::detail::TextureVulkanState init_texture_state
    {
        vk::QueueFamilyIgnored,
        vk::AccessFlagBits2::eNone,
        vk::PipelineStageFlagBits2::eAllCommands,
        vk::ImageLayout::eUndefined
    };

    inline constexpr caldera::detail::BufferVulkanState init_buffer_state
    {
        vk::QueueFamilyIgnored,
        vk::AccessFlagBits2::eNone,
        vk::PipelineStageFlagBits2::eAllCommands
    };

    template <class StateTy>
    [[nodiscard]] bool needs_transition(
        StateTy const previousLowLevelState,
        StateTy const requiredLowLevelState)
    {
        using namespace caldera;

        if constexpr (std::same_as<std::remove_reference_t<StateTy>, detail::TextureVulkanState>)
        {
            if (previousLowLevelState == init_texture_state)
                return true;

            if (previousLowLevelState.layout == vk::ImageLayout::eUndefined ||
                previousLowLevelState.layout != requiredLowLevelState.layout)
            {
                return true;
            }
        }
        else {
            if (previousLowLevelState == init_buffer_state)
                return true;
        }

        if (previousLowLevelState.family != requiredLowLevelState.family)
            return true;

        auto constexpr write_mask =
            vk::AccessFlagBits2::eShaderWrite |
            vk::AccessFlagBits2::eColorAttachmentWrite |
            vk::AccessFlagBits2::eDepthStencilAttachmentWrite |
            vk::AccessFlagBits2::eTransferWrite |
            vk::AccessFlagBits2::eMemoryWrite;

        auto const previousWrites = static_cast<bool>(previousLowLevelState.access & write_mask);
        auto const requiredWrites = static_cast<bool>(requiredLowLevelState.access & write_mask);

        if (previousWrites || requiredWrites)
            return true;

        return false;
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
        }
    }

    detail::TextureVulkanState RenderGraph::gen_low_level_state(detail::TextureState const state) const
    {
        if (state.queue == QueueType::none)
            return init_texture_state;

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

    detail::BufferVulkanState RenderGraph::gen_low_level_state(detail::BufferState const state) const
    {
        if (state.queue == QueueType::none)
            return init_buffer_state;

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

    TextureID RenderGraph::declare_texture(vk::ImageAspectFlags const aspects)
    {
        auto const index = static_cast<uint32_t>(m_textures.size());
        auto& desc = m_textures.emplace_back();

        desc.aspects = aspects;
        return TextureID{ index };
    }

    BufferID RenderGraph::declare_buffer(vk::DeviceSize const size)
    {
        auto const index = static_cast<uint32_t>(m_buffers.size());
        auto& desc = m_buffers.emplace_back();

        desc.size = size;
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

        textureStates.resize(m_textures.size(), init_texture_state);
        bufferStates.resize(m_buffers.size(), init_buffer_state);

        for (auto& pass : m_passes)
        {
            for (auto const [id, state] : pass.m_textureBindings)
            {
                auto& prevState = textureStates[id.id];
                auto reqState = gen_low_level_state(state);

                if (prevState.family == vk::QueueFamilyIgnored)
                    reqState.family = vk::QueueFamilyIgnored;

                if (needs_transition(prevState, reqState))
                {
                    auto const subresource = make_subresource(state, m_textures[id.id].aspects);
                    assert(static_cast<bool>(subresource.aspectMask));

                    pass.m_textureTransitions.emplace_back(id, prevState, reqState, subresource);
                    prevState = reqState;
                }
                else {
                    prevState.access |= reqState.access;
                    prevState.stages |= reqState.stages;
                }
            }

            for (auto const [id, state] : pass.m_bufferBindings)
            {
                auto& prevState = bufferStates[id.id];
                auto reqState = gen_low_level_state(state);

                if (prevState.family == vk::QueueFamilyIgnored)
                    reqState.family = vk::QueueFamilyIgnored;

                if (needs_transition(prevState, reqState))
                {
                    pass.m_bufferTransitions.emplace_back(id, prevState, reqState);
                    prevState = reqState;
                }
                else {
                    prevState.access |= reqState.access;
                    prevState.stages |= reqState.stages;
                }
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
