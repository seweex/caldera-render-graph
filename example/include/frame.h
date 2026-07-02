#ifndef CALDERA_EXAMPLE_FRAME_H
#define CALDERA_EXAMPLE_FRAME_H

#include <vulkan/vulkan.hpp>

namespace caldera_example
{
    struct Device;
    struct Swapchain;

    struct FrameContext
    {
    private:
        [[nodiscard]] static vk::CommandPool create_pool(vk::Device device, uint32_t family);
        [[nodiscard]] static vk::CommandBuffer allocate_buffer(vk::Device device, vk::CommandPool pool);

        [[nodiscard]] static vk::Semaphore create_semaphore(vk::Device device, bool timeline);

    public:
        FrameContext() noexcept;
        ~FrameContext() noexcept;

        FrameContext(FrameContext &&) noexcept;
        FrameContext& operator=(FrameContext &&) noexcept;

        FrameContext(FrameContext const&) = delete;
        FrameContext& operator=(FrameContext const&) = delete;

        [[nodiscard]] bool init(Device const& device);
        void clear() noexcept;

    private:
        vk::Device m_device;

    public:
        vk::CommandPool pool;
        vk::CommandBuffer buffer;

        vk::Semaphore timelineSemaphore;
        uint64_t timelineValue;

        vk::Semaphore imageAvailableSemaphore;
        vk::Semaphore renderFinishedSemaphore;
    };

    struct FrameManager
    {
        FrameManager() noexcept;
        ~FrameManager() noexcept;

        FrameManager(FrameManager &&) = delete;
        FrameManager& operator=(FrameManager &&) = delete;

        FrameManager(FrameManager const&) = delete;
        FrameManager& operator=(FrameManager const&) = delete;

        [[nodiscard]] bool init(Device const& device, Swapchain const& swapchain);
        void clear() noexcept;

        std::vector<FrameContext> frames;
    };
}

#endif