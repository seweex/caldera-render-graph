#ifndef CALDERA_EXAMPLE_FRAME_H
#define CALDERA_EXAMPLE_FRAME_H

#include <vulkan_include.h>

namespace caldera_example
{
    struct FrameResources
    {
        FrameResources() noexcept;
        ~FrameResources() noexcept;

        FrameResources(FrameResources &&) = delete;
        FrameResources& operator=(FrameResources &&) = delete;

        FrameResources(FrameResources const&) = delete;
        FrameResources& operator=(FrameResources const&) = delete;

        [[nodiscard]] bool init(
            vk::Device device,
            uint32_t family,
            uint32_t bufferCount);

        void clear(vk::Device device) noexcept;

        vk::CommandPool pool;
        std::vector<vk::CommandBuffer> buffers;

        uint32_t activeBufferIndex;
        uint64_t previousSubmissionTicket;

        vk::Semaphore imageAvailableSemaphore;
        vk::Semaphore renderFinishedSemaphore;
    };
}

#endif