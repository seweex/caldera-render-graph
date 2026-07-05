#ifndef CALDERA_EXAMPLE_FRAME_H
#define CALDERA_EXAMPLE_FRAME_H

#include <array>

#include <vulkan_include.h>

namespace caldera_example
{
    struct Device;
    struct Swapchain;

    struct FrameContext
    {
    private:
        [[nodiscard]] static vk::CommandPool create_pool(vk::Device device, uint32_t family);
        [[nodiscard]] static vk::CommandBuffer allocate_buffer(vk::Device device, vk::CommandPool pool);

    public:
        FrameContext() noexcept;
        ~FrameContext() noexcept;

        FrameContext(FrameContext &&) = delete;
        FrameContext& operator=(FrameContext &&) = delete;

        FrameContext(FrameContext const&) = delete;
        FrameContext& operator=(FrameContext const&) = delete;

        [[nodiscard]] bool init(Device const& device);
        void clear(vk::Device device) noexcept;

        vk::CommandPool pool;
        vk::CommandBuffer buffer;

        vk::Semaphore imageAvailableSemaphore;
        vk::Semaphore renderFinishedSemaphore;
    };

    struct FrameManager
    {
        static constexpr uint32_t frames_inflight = 3;

        FrameManager() noexcept;
        ~FrameManager() noexcept;

        FrameManager(FrameManager &&) = delete;
        FrameManager& operator=(FrameManager &&) = delete;

        FrameManager(FrameManager const&) = delete;
        FrameManager& operator=(FrameManager const&) = delete;

        [[nodiscard]] bool init(Device const& device);
        void clear() noexcept;

        void advance() noexcept;

    private:
        vk::Device m_device;

    public:
        vk::Semaphore timelineSemaphore;
        uint64_t timelineValue;

        std::array<FrameContext, frames_inflight> frames;
        uint32_t currentFrame;
    };
}

#endif