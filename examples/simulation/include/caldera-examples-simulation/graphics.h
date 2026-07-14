#ifndef CALDERA_EXAMPLE_SIMULATION_GRAPHICS_H
#define CALDERA_EXAMPLE_SIMULATION_GRAPHICS_H

#include <caldera-render-graph/graph.h>
#include <caldera-render-graph/pass.h>

#include <caldera-examples-common/window.h>
#include <caldera-examples-common/device.h>
#include <caldera-examples-common/swapchain.h>
#include <caldera-examples-common/scheduler.h>
#include <caldera-examples-common/memory.h>
#include <caldera-examples-common/buffer.h>
#include <caldera-examples-common/image.h>
#include <caldera-examples-common/shader.h>
#include <caldera-examples-common/pipeline.h>

#include <caldera-examples-simulation/simulation.h>
#include <caldera-examples-simulation/pipeline.h>
#include <caldera-examples-simulation/descriptors.h>

#include <vulkan/vulkan_raii.hpp>

namespace caldera_examples_simulation
{
    namespace common = caldera_examples_common;

    struct GraphicsContext
    {
        [[nodiscard]] bool init();

        common::Context context;
        common::Window window;
        common::Device device;
        common::Swapchain swapchain;
        common::Scheduler scheduler;
        common::Allocator allocator;

        common::Shader computeShader;
        common::Shader vertexShader;
        common::Shader fragmentShader;

        ComputeLayout computeLayout;
        SettingsLayout graphicsLayout;

        ComputePipeline computePipeline;
        GraphicsPipeline graphicsPipeline;
    };

    struct GraphicsResources
    {
        [[nodiscard]] bool init(GraphicsContext& context);

        common::Buffer staging;
        common::Buffer planets;

        ComputeDescriptors descriptors;

        std::vector<std::unique_ptr<caldera_examples_common::Image>> depthImages;
        std::vector<vk::UniqueImageView> depthViews;
    };

    struct FrameGraphs
    {
        struct Loads {
            caldera::RenderGraph graph;
            caldera::BufferID staging;
            caldera::BufferID planets;
        };

        struct Simulation {
            caldera::RenderGraph graph;
            caldera::BufferID planets;
            caldera::TextureID depth;
            caldera::TextureID swapchain;
        };

        [[nodiscard]] bool init(
            GraphicsContext& context,
            GraphicsResources& resources,
            Settings& settings,
            std::vector<Planet> planets);

        std::optional<Loads> loads;
        std::optional<Simulation> simulation;
    };
}

#endif