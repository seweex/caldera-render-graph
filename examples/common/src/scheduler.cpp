
#include <caldera-examples-common/scheduler.h>
#include <caldera-examples-common/device.h>
#include <caldera-examples-common/swapchain.h>

#include <spdlog/spdlog.h>

namespace
{
    [[nodiscard]] vk::Semaphore create_timeline_semaphore(vk::Device const device)
    {
        vk::StructureChain const createInfo
        {
            vk::SemaphoreCreateInfo {
                vk::SemaphoreCreateFlags{}
            },
            vk::SemaphoreTypeCreateInfo {
                vk::SemaphoreType::eTimeline, 0
            }
        };

        auto const newSemaphore = device.createSemaphore(createInfo.get<>());

        if (!newSemaphore.has_value()) {
            spdlog::error("Failed to create a semaphore: {}", vk::to_string(newSemaphore.result));
            return VK_NULL_HANDLE;
        }

        return *newSemaphore;
    }

    [[nodiscard]] bool init_frames(
        auto& frames,
        vk::Device const device,
        uint32_t const family,
        uint32_t const buffers)
    {
        for (uint32_t i = 0; i < frames.size(); ++i)
            if (!frames[i].init(device, family, buffers))
            {
                for (uint32_t j = 0; j < i; ++j)
                    frames[j].clear(device);

                return false;
            }

        return true;
    }
}

namespace caldera_examples_common
{
    Scheduler::Scheduler() noexcept = default;

    Scheduler::~Scheduler() noexcept {
        clear();
    }

    bool Scheduler::init(
        Device const& device,
        Swapchain const& swapchain,
        uint32_t const commandBuffersPerFrame)
    {
        m_device = device.device;
        m_queue = m_device.getQueue(device.queueFamilyIndex, 0);
        m_swapchain = swapchain.swapchain;

        timelineValue = 0;
        currentFrame = 0;

        if (!(timelineSemaphore = create_timeline_semaphore(m_device)) ||
            !init_frames(frames, m_device, device.queueFamilyIndex, commandBuffersPerFrame))
        {
            clear();
            return false;
        }

        return true;
    }

    void Scheduler::clear() noexcept
    {
        if (m_device)
        {
            for (auto& frame : frames)
                frame.clear(m_device);

            m_device.destroySemaphore(timelineSemaphore);

            timelineSemaphore = VK_NULL_HANDLE;
            m_device = VK_NULL_HANDLE;
        }
    }

    bool Scheduler::begin_frame()
    {
        auto& frame = frames[currentFrame];

        /* Wait previous frame */
        if (!wait_for_ticket(frame.previousSubmissionTicket))
            return false;

        /* Wait for presentation */
        {
            if (auto const result = m_device.waitForFences(frame.imagePresentedFence, vk::True, UINT64_MAX);
               result < vk::Result::eSuccess)
            {
                spdlog::error("Failed to wait for a fence: {}", vk::to_string(result));
                return false;
            }

            if (auto const result = m_device.resetFences(frame.imagePresentedFence);
               result < vk::Result::eSuccess)
            {
                spdlog::error("Failed to reset a fence: {}", vk::to_string(result));
                return false;
            }
        }

        /* Acquire image */
        if (auto const result = m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX,
                frame.imageAvailableSemaphore, VK_NULL_HANDLE, &currentImage);
            result < vk::Result::eSuccess)
        {
            spdlog::error("Failed to acquire an image: {}", vk::to_string(result));
            return false;
        }

        /* Reset used resource */
        {
            if (auto const result = m_device.resetCommandPool(frame.pool);
               result < vk::Result::eSuccess)
            {
                spdlog::error("Failed to reset command pool: {}", vk::to_string(result));
                return false;
            }

            frame.activeBufferIndex = 0;
        }

        return true;
    }

    bool Scheduler::end_frame()
    {
        auto& frame = frames[currentFrame];

        /* Present image */
        {
            vk::StructureChain const presentInfo
            {
                vk::PresentInfoKHR{
                    frame.renderFinishedSemaphore, m_swapchain, currentImage
                },
                vk::SwapchainPresentFenceInfoEXT {
                    1, &frame.imagePresentedFence
                }
            };

            if (auto const result = m_queue.presentKHR(presentInfo.get());
                result < vk::Result::eSuccess)
            {
                spdlog::error("Failed to present an image: {}", vk::to_string(result));
                return false;
            }
        }

        /* Advance frame */
        currentFrame = (currentFrame + 1) % frames_inflight;

        return true;
    }

    vk::CommandBuffer Scheduler::get_current_command_buffer() noexcept
    {
        auto const& frame = frames[currentFrame];
        return frame.buffers[frame.activeBufferIndex];
    }

    uint64_t Scheduler::submit_current_buffer(uint64_t const ticketToWait, bool const presentAfterIt)
    {
        auto const resultTicket = timelineValue + 1;
        auto& frame = frames[currentFrame];

        std::array<vk::SemaphoreSubmitInfo, 2> waitInfo;
        std::array<vk::SemaphoreSubmitInfo, 2> signalInfo;

        uint32_t waitCount = 0;
        uint32_t signalCount = 0;

        if (ticketToWait)
        {
            waitInfo[waitCount].stageMask = vk::PipelineStageFlagBits2::eAllCommands;
            waitInfo[waitCount].semaphore = timelineSemaphore;
            waitInfo[waitCount].value = ticketToWait;
            ++waitCount;
        }

        signalInfo[signalCount].stageMask = vk::PipelineStageFlagBits2::eAllCommands;
        signalInfo[signalCount].semaphore = timelineSemaphore;
        signalInfo[signalCount].value = resultTicket;
        ++signalCount;

        if (presentAfterIt)
        {
            waitInfo[waitCount].stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
            waitInfo[waitCount].semaphore = frame.imageAvailableSemaphore;
            ++waitCount;

            signalInfo[signalCount].stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
            signalInfo[signalCount].semaphore = frame.renderFinishedSemaphore;
            ++signalCount;

            frame.previousSubmissionTicket = resultTicket;
        }

        vk::CommandBufferSubmitInfo const bufferInfo
            { frame.buffers[frame.activeBufferIndex] };

        vk::SubmitInfo2 const submitInfo {
            vk::SubmitFlags{},
            waitCount, waitInfo.data(),
            1, &bufferInfo,
            signalCount, signalInfo.data()
        };

        if (auto const result = m_queue.submit2(submitInfo);
            result < vk::Result::eSuccess)
        {
            spdlog::error("Failed to submit a command buffer: {}", vk::to_string(result));
            return 0;
        }

        ++timelineValue;
        ++frame.activeBufferIndex;

        return resultTicket;
    }

    bool Scheduler::wait_for_ticket(uint64_t const ticket)
    {
        if (ticket == 0)
            return true;

        vk::SemaphoreWaitInfo const waitInfo
            { vk::SemaphoreWaitFlags{}, 1, &timelineSemaphore, &ticket };

        if (auto const result = m_device.waitSemaphores(waitInfo, UINT64_MAX);
            result < vk::Result::eSuccess)
        {
            spdlog::error("Failed to wait for semaphore: {}", vk::to_string(result));
            return false;
        }

        return true;
    }

    bool Scheduler::wait_idle()
    {
        if (auto const result = m_queue.waitIdle();
            result < vk::Result::eSuccess)
        {
            spdlog::error("Failed to wait queue idle: {}", vk::to_string(result));
            return false;
        }

        return true;
    }
}
