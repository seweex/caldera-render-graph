
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <random>
#include <GLFW/glfw3.h>

#include <caldera-examples-simulation/graphics.h>
#include <caldera-examples-simulation/simulation.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

[[nodiscard]] glm::mat4 calc_camera_matrix() noexcept
{
    auto constexpr camera_basic_position = glm::vec3(-100.0f, 45.0f, -100.0f);
    auto constexpr camera_look = glm::vec3(0.f, -10.f, 0.f);
    auto constexpr camera_fov = 45.f;

    auto const view = glm::lookAt(camera_basic_position, camera_look, glm::vec3(0.0f, -1.0f, 0.0f));
    auto const proj = glm::perspective(glm::radians(camera_fov), 16.f / 9.f, 0.01f, 1000000.f);

    return proj * view;
}

[[nodiscard]] std::vector<caldera_examples_simulation::Planet> gen_planets()
{
    std::vector<caldera_examples_simulation::Planet> result;
    result.resize(caldera_examples_simulation::planet_count);

    result[0].position_and_mass = glm::vec4(0.f, 0.f, 0.f, 10000.f);
    result[0].color_and_radius = glm::vec4(1.f, 1.f, 1.f, 1000.f);
    result[0].speed = glm::vec4(0.f);

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    for (uint32_t i = 1; i < caldera_examples_simulation::planet_count; ++i)
    {
        auto& planet = result[i];

        auto const orbitRadius = 10.0f + (static_cast<float>(i) / caldera_examples_simulation::planet_count) * 70.0f + dist01(rng) * 2.0f;
        auto const angle = dist01(rng) * glm::two_pi<float>();
        auto const inclination = (dist01(rng) - 0.5f) * 2.0f;

        planet.position_and_mass.x = orbitRadius * glm::cos(angle);
        planet.position_and_mass.y = inclination;
        planet.position_and_mass.z = orbitRadius * glm::sin(angle);
        planet.position_and_mass.w = 0.1f + dist01(rng) * 10.0f;

        planet.color_and_radius.r = 0.2f + dist01(rng) * 0.8f;
        planet.color_and_radius.g = 0.2f + dist01(rng) * 0.8f;
        planet.color_and_radius.b = 0.2f + dist01(rng) * 0.8f;
        planet.color_and_radius.w = 300.f + dist01(rng) * 150.f;

        auto const orbitalSpeed = glm::sqrt((caldera_examples_simulation::gravity * result[0].position_and_mass.w) / orbitRadius);

        planet.speed.x = -orbitalSpeed * glm::sin(angle);
        planet.speed.y = 0.0f;
        planet.speed.z = orbitalSpeed * glm::cos(angle);
    }

    return result;
}

int main()
{
    using namespace caldera_examples_simulation;

    GraphicsContext context;
    GraphicsResources resources;
    FrameGraphs graphs;

    Settings settings;
    settings.cameraMatrix = calc_camera_matrix();
    settings.timeSpeed = 2.0f;
    settings.deltaTime = 0.1f;

    if (!context.init() ||
        !resources.init(context) ||
        !graphs.init(context, resources, settings, gen_planets()))
    {
        return 1;
    }

    double currentTime = glfwGetTime();
    bool firstFrame = true;

    while (!context.window.closing())
    {
        caldera_examples_common::Window::poll_events();

        if (!context.scheduler.begin_frame())
            return 1;

        uint64_t ticketToWait = 0;

        if (firstFrame)
        {
            auto const cmd = context.scheduler.get_current_command_buffer();
            (void)cmd.begin(vk::CommandBufferBeginInfo{});

            graphs.loads->graph.associate(graphs.loads->staging, resources.staging.buffer);
            graphs.loads->graph.associate(graphs.loads->planets, resources.planets.buffer);
            graphs.loads->graph.execute(cmd);

            (void)cmd.end();

            ticketToWait = context.scheduler.submit_current_buffer(0, false);
            firstFrame = false;
        }

        auto const cmd = context.scheduler.get_current_command_buffer();
        (void)cmd.begin(vk::CommandBufferBeginInfo{});

        graphs.simulation->graph.associate(
            graphs.simulation->depth,
            resources.depthImages[context.scheduler.currentImage]->image,
            resources.depthViews[context.scheduler.currentImage].get());

        graphs.simulation->graph.associate(
            graphs.simulation->swapchain,
            context.swapchain.images[context.scheduler.currentImage],
            context.swapchain.imageViews[context.scheduler.currentImage]);

        graphs.simulation->graph.execute(cmd);
        (void)cmd.end();

        if (0 == context.scheduler.submit_current_buffer(ticketToWait, true) ||
            !context.scheduler.end_frame())
            return 1;

        double endTime = glfwGetTime();
        settings.deltaTime = static_cast<float>(endTime - currentTime);
        currentTime = endTime;
    }

    return context.scheduler.wait_idle() ? 0 : 1;
}
