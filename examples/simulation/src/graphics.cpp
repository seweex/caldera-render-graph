
#include <caldera-examples-common/descriptor.h>

#include <caldera-examples-simulation/graphics.h>
#include <caldera-examples-simulation/simulation.h>

#include <compiled-shaders/gravity_simulation.comp.hpp>
#include <compiled-shaders/simulation.vert.hpp>
#include <compiled-shaders/simulation.frag.hpp>

namespace
{
    [[nodiscard]] std::vector<std::unique_ptr<caldera_examples_common::Image>>
    create_depth_images(caldera_examples_simulation::GraphicsContext& context)
    {
        std::vector<std::unique_ptr<caldera_examples_common::Image>> result;
        result.resize(context.swapchain.images.size());

        caldera_examples_common::Image::Settings constexpr settings {
            vk::Format::eD32Sfloat,
            vk::Extent2D{ 1024, 576 },
            vk::ImageUsageFlagBits::eDepthStencilAttachment
        };

        for (auto& image : result)
        {
            image = std::make_unique<caldera_examples_common::Image>();

            if (!image->init(context.device, context.allocator, settings))
                return {};
        }

        return result;
    }

    [[nodiscard]] std::vector<vk::UniqueImageView>
    create_depth_views(
        caldera_examples_simulation::GraphicsContext& context,
        std::vector<std::unique_ptr<caldera_examples_common::Image>> const& images)
    {
        std::vector<vk::UniqueImageView> result;
        result.reserve(images.size());

        for (auto const& image : images)
        {
            vk::ImageViewCreateInfo const info {
                vk::ImageViewCreateFlags{},
                image->image,
                vk::ImageViewType::e2D,
                vk::Format::eD32Sfloat,
                vk::ComponentMapping{},
                vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 }
            };

            if (auto newView = context.device.device.createImageViewUnique(info);
                !newView.has_value())
            {
                return {};
            }
            else
                result.emplace_back(std::move(*newView));
        }

        return result;
    }

    [[nodiscard]] caldera_examples_simulation::FrameGraphs::Loads
    make_load_graph(
        caldera_examples_simulation::GraphicsContext& context,
        caldera_examples_simulation::GraphicsResources& resources,
        std::vector<caldera_examples_simulation::Planet> planets)
    {
        caldera::RenderGraph graph {
            context.device.device,
            context.device.queueFamilyIndex,
            context.device.queueFamilyIndex,
            context.device.queueFamilyIndex
        };

        auto constexpr size =
            sizeof(caldera_examples_simulation::Planet) *
            caldera_examples_simulation::planet_count;

        auto const stagingID = graph.declare_buffer(size);
        auto const planetID = graph.declare_buffer(size);

        caldera::PassNode stagingPass{ "staging-pass", caldera::QueueType::transfer };
        stagingPass.write(stagingID, caldera::BufferUsage::mapped_usage);
        stagingPass.callback([pls = std::move(planets), &resources] (vk::CommandBuffer)
        {
            auto const mapping = resources.staging.get_constant_mapping();
            std::memcpy(mapping, pls.data(), size);
        });

        caldera::PassNode transferPass{ "transfer-pass", caldera::QueueType::transfer };
        transferPass.read(stagingID, caldera::BufferUsage::transfer);
        transferPass.write(planetID, caldera::BufferUsage::transfer);
        transferPass.callback([&resources] (vk::CommandBuffer const cmd)
        {
            vk::BufferCopy2 constexpr copyInfo { 0, 0, size };

            cmd.copyBuffer2(vk::CopyBufferInfo2{
                resources.staging.buffer,
                resources.planets.buffer,
                copyInfo });
        });

        graph.push_pass(std::move(stagingPass));
        graph.push_pass(std::move(transferPass));

        graph.compile();

        return { std::move(graph), stagingID, planetID };
    }

    [[nodiscard]] caldera_examples_simulation::FrameGraphs::Simulation
    make_simulation_graph(
        caldera_examples_simulation::GraphicsContext& context,
        caldera_examples_simulation::GraphicsResources& resources,
        caldera_examples_simulation::Settings& settings)
    {
        caldera::RenderGraph graph {
            context.device.device,
            context.device.queueFamilyIndex,
            context.device.queueFamilyIndex,
            context.device.queueFamilyIndex
        };

        auto constexpr size =
            sizeof(caldera_examples_simulation::Planet) *
            caldera_examples_simulation::planet_count;

        auto const planetsID = graph.declare_buffer(size);
        auto const depthID = graph.declare_texture(vk::ImageAspectFlagBits::eDepth);
        auto const swapchainID = graph.declare_texture(vk::ImageAspectFlagBits::eColor);

        caldera::PassNode gravityPass{ "gravity-pass", caldera::QueueType::compute };
        gravityPass.write(planetsID, caldera::BufferUsage::mapped_usage);
        gravityPass.callback([&context, &resources, &settings] (vk::CommandBuffer const cmd)
        {
            cmd.bindDescriptorSets(
                vk::PipelineBindPoint::eCompute,
                context.computeLayout.pipelineLayout,
                0,
                1,
                &resources.descriptors.set,
                0,
                nullptr);

            cmd.pushConstants(
                context.computeLayout.pipelineLayout,
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment,
                0,
                sizeof(caldera_examples_simulation::Settings),
                &settings);

            cmd.bindPipeline(
                vk::PipelineBindPoint::eCompute,
                context.computePipeline.pipeline);

            cmd.dispatch(1, 1, 1);
        });

        caldera::PassNode drawPass{ "draw-pass", caldera::QueueType::graphics };
        drawPass.read(planetsID, caldera::BufferUsage::mapped_usage);
        drawPass.write(swapchainID, caldera::TextureUsage::color_attachment);
        drawPass.write(depthID, caldera::TextureUsage::depth_attachment);
        drawPass.callback([&context, &resources, &settings] (vk::CommandBuffer const cmd)
        {
            vk::DeviceSize constexpr offset = 0;

            cmd.pushConstants(
                context.graphicsLayout.pipelineLayout,
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment,
                0,
                sizeof(caldera_examples_simulation::Settings),
                &settings);

            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, context.graphicsPipeline.pipeline);
            cmd.bindVertexBuffers(0, 1, &resources.planets.buffer, &offset);

            vk::RenderingAttachmentInfo const colorAttachment
            {
                context.swapchain.imageViews[context.scheduler.currentImage],
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::ResolveModeFlagBits::eNone,
                VK_NULL_HANDLE,
                vk::ImageLayout::eUndefined,
                vk::AttachmentLoadOp::eClear,
                vk::AttachmentStoreOp::eStore,
                vk::ClearColorValue{ 0.f, 0.f, 0.f, 1.f },
            };

            vk::RenderingAttachmentInfo const depthAttachment
            {
                resources.depthViews[context.scheduler.currentImage].get(),
                vk::ImageLayout::eDepthStencilAttachmentOptimal,
                vk::ResolveModeFlagBits::eNone,
                VK_NULL_HANDLE,
                vk::ImageLayout::eUndefined,
                vk::AttachmentLoadOp::eClear,
                vk::AttachmentStoreOp::eDontCare,
                vk::ClearDepthStencilValue{ 1.f, 0 },
            };

            vk::RenderingInfo const renderingInfo
            {
                vk::RenderingFlags{},
                vk::Rect2D{ { 0, 0 }, { 1024, 576 } },
                1, 0,
                1, &colorAttachment,
                &depthAttachment,
                nullptr
            };

            cmd.beginRendering(renderingInfo);
            cmd.draw(caldera_examples_simulation::planet_count, 1, 0, 0);
            cmd.endRendering();
        });

        caldera::PassNode presentPass{ "present-pass", caldera::QueueType::graphics };
        presentPass.read(swapchainID, caldera::TextureUsage::present);
        presentPass.callback([] (vk::CommandBuffer) {});

        graph.push_pass(std::move(gravityPass));
        graph.push_pass(std::move(drawPass));
        graph.push_pass(std::move(presentPass));

        graph.associate(planetsID, resources.planets.buffer);
        graph.compile();

        return { std::move(graph), planetsID, depthID, swapchainID };
    }
}

namespace caldera_examples_simulation
{
    bool GraphicsContext::init()
    {
        if (!context.init() ||
            !window.init(context) ||
            !device.init(context, window) ||
            !swapchain.init(device, window) ||
            !scheduler.init(device, swapchain, 2) ||
            !allocator.init(context, device) ||
            !computeShader.init(device, shader_link_compiled::spv_gravity_simulation_comp) ||
            !vertexShader.init(device, shader_link_compiled::spv_simulation_vert) ||
            !fragmentShader.init(device, shader_link_compiled::spv_simulation_frag) ||
            !computeLayout.init(device) ||
            !graphicsLayout.init(device) ||
            !computePipeline.init(device, computeLayout, computeShader) ||
            !graphicsPipeline.init(window, device, graphicsLayout, vertexShader, fragmentShader))
        {
            return false;
        }

        return true;
    }

    bool GraphicsResources::init(GraphicsContext& context)
    {
        common::Buffer::Settings constexpr staging_settings {
            sizeof(Planet) * planet_count,
            common::Buffer::MemoryType::constantly_mapped,
            vk::BufferUsageFlagBits::eTransferSrc
        };

        common::Buffer::Settings constexpr planets_settings {
            sizeof(Planet) * planet_count,
            common::Buffer::MemoryType::gpu_local,
            vk::BufferUsageFlagBits::eTransferDst |
            vk::BufferUsageFlagBits::eStorageBuffer |
            vk::BufferUsageFlagBits::eVertexBuffer
        };

        if (!staging.init(context.device, context.allocator, staging_settings) ||
            !planets.init(context.device, context.allocator, planets_settings) ||
            !descriptors.init(context.device, context.computeLayout) ||
            (depthImages = create_depth_images(context)).empty() ||
            (depthViews = create_depth_views(context, depthImages)).empty())
        {
            return false;
        }

        descriptors.bind(planets.buffer);
        return true;
    }

    bool FrameGraphs::init(
        GraphicsContext& context,
        GraphicsResources& resources,
        Settings& settings,
        std::vector<Planet> planets)
    {
        loads.emplace(make_load_graph(context, resources, std::move(planets)));
        simulation.emplace(make_simulation_graph(context, resources, settings));

        return true;
    }
}
