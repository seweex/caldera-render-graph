#ifndef CALDERA_EXAMPLE_SCHEDULER_H
#define CALDERA_EXAMPLE_SCHEDULER_H

#include <frame.h>

#include <vulkan_include.h>
#include <array>

namespace caldera_example
{
    struct Device;
    struct Swapchain;

    struct Scheduler
    {
        static constexpr uint32_t frames_inflight = 3;

        Scheduler() noexcept;
        ~Scheduler() noexcept;

        Scheduler(Scheduler &&) = delete;
        Scheduler& operator=(Scheduler &&) = delete;

        Scheduler(Scheduler const&) = delete;
        Scheduler& operator=(Scheduler const&) = delete;

        [[nodiscard]] bool init(
            Device const& device,
            Swapchain const& swapchain,
            uint32_t commandBuffersPerFrame);

        void clear() noexcept;

        [[nodiscard]] bool begin_frame();
        [[nodiscard]] bool end_frame();

        [[nodiscard]] vk::CommandBuffer get_current_command_buffer() noexcept;
        [[nodiscard]] uint64_t submit_current_buffer(uint64_t ticketToWait, bool presentAfterIt);

        [[nodiscard]] bool wait_for_ticket(uint64_t ticket);
        [[nodiscard]] bool wait_idle();

    private:
        vk::Device m_device;
        vk::Queue m_queue;

        vk::SwapchainKHR m_swapchain;

    public:
        vk::Semaphore timelineSemaphore;
        uint64_t timelineValue;

        std::array<FrameResources, frames_inflight> frames;

        uint32_t currentFrame;
        uint32_t currentImage;
    };
}

#endif
