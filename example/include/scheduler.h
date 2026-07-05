#ifndef CALDERA_EXAMPLE_SCHEDULER_H
#define CALDERA_EXAMPLE_SCHEDULER_H

#include <vulkan_include.h>
#include <frame.h>

namespace caldera_example
{
    struct Device;
    struct Swapchain;

    struct Scheduler
    {
        Scheduler() noexcept;
        ~Scheduler() noexcept;

        Scheduler(Scheduler &&) = delete;
        Scheduler& operator=(Scheduler &&) = delete;

        Scheduler(Scheduler const&) = delete;
        Scheduler& operator=(Scheduler const&) = delete;

        [[nodiscard]] bool init(Device const& device);
        void clear() noexcept;

        [[nodiscard]] bool begin_frame(Swapchain const& swapchain);
        [[nodiscard]] bool end_frame(Swapchain const& swapchain);

        [[nodiscard]] vk::CommandBuffer get_current_command_buffer() noexcept;

    private:
        vk::Device m_device;
        vk::Queue m_queue;

        FrameManager m_frameManager;
        uint32_t m_currentImage;
    };
}

#endif
