
#include <scheduler.h>
#include <device.h>
#include <swapchain.h>
#include <spdlog/spdlog.h>

namespace caldera_example
{
    std::optional<uint32_t> Scheduler::acquire_next_image(vk::SwapchainKHR swapchain)
    {
        auto const semaphore = m_frameManager.frames[m_frameManager.currentFrame].imageAvailableSemaphore;
        auto const image = m_device.acquireNextImageKHR(
            swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE);

        if (!image.has_value()) {
            spdlog::error("Failed to acquire a swapchain image: ", vk::to_string(image.result));
            return std::nullopt;
        }

        return *image;
    }

    bool Scheduler::present_image(vk::SwapchainKHR swapchain)
    {
        auto const semaphore = m_frameManager.frames[m_frameManager.currentFrame].renderFinishedSemaphore;

        vk::PresentInfoKHR const presentInfo {
            1, &semaphore,
            1, &swapchain, &m_currentImage
        };

        if (auto const result = m_queue.presentKHR(presentInfo);
            result < vk::Result::eSuccess)
        {
            spdlog::error("Failed to present an image: ", vk::to_string(result));
            return false;
        }

        return true;
    }

    bool Scheduler::wait_previous_frame_timeline()
    {
        if (m_frameManager.timelineValue <= FrameManager::frames_inflight)
            return true;

        auto const waitValue = m_frameManager.timelineValue - FrameManager::frames_inflight;

        vk::SemaphoreWaitInfo const waitInfo
        {
            vk::SemaphoreWaitFlags{},
            1, &m_frameManager.timelineSemaphore, &waitValue
        };

        if (auto const result = m_device.waitSemaphores(waitInfo, UINT64_MAX);
            result < vk::Result::eSuccess)
        {
            spdlog::error("Failed to wait for the timeline semaphore: {}", vk::to_string(result));
            return false;
        }

        return true;
    }

    bool Scheduler::submit_commands()
    {
        auto const& currentFrame = m_frameManager.frames[m_frameManager.currentFrame];

        vk::SemaphoreSubmitInfo const waitInfo
        {
            currentFrame.imageAvailableSemaphore,
            0,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            0
        };

        std::array const signalInfos
        {
            vk::SemaphoreSubmitInfo
            {
                currentFrame.renderFinishedSemaphore,
                0,
                vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                0
            },
            vk::SemaphoreSubmitInfo
            {
                m_frameManager.timelineSemaphore,
                m_frameManager.timelineValue,
                vk::PipelineStageFlagBits2::eAllCommands,
                0
            }
        };

        vk::CommandBufferSubmitInfo const bufferInfo {
            currentFrame.buffer
        };

        vk::SubmitInfo2 const submitInfo
        {
            vk::SubmitFlags{},
            waitInfo,
            bufferInfo,
            signalInfos
        };

        if (auto const result = m_queue.submit2(1, &submitInfo, VK_NULL_HANDLE);
            result < vk::Result::eSuccess)
        {
            spdlog::error("Failed to submit commands: {}", vk::to_string(result));
            return false;
        }

        return true;
    }

    Scheduler::Scheduler() noexcept = default;

    Scheduler::~Scheduler() noexcept {
        clear();
    }

    bool Scheduler::init(Device const& device)
    {
        m_device = device.device;
        m_queue = m_device.getQueue(device.queueFamilyIndex, 0);

        if (!m_frameManager.init(device))
            return false;

        return true;
    }

    void Scheduler::clear() noexcept
    {
        if (auto const result = m_queue.waitIdle();
            result < vk::Result::eSuccess)
        {
            spdlog::error("Failed to wait for queue idle: {}", vk::to_string(result));
        }

        m_frameManager.clear();
    }

    bool Scheduler::begin_frame(Swapchain const& swapchain)
    {
        ++m_frameManager.timelineValue;

        if (!wait_previous_frame_timeline())
            return false;

        auto const image = acquire_next_image(swapchain.swapchain);

        if (!image.has_value() ||
            !m_frameManager.reset_current_pool())
        {
            return false;
        }

        m_currentImage = *image;
        return true;
    }

    bool Scheduler::end_frame(Swapchain const& swapchain)
    {
        if (!submit_commands() ||
            !present_image(swapchain.swapchain))
        {
            return false;
        }

        m_frameManager.advance();
        return true;
    }

    vk::CommandBuffer Scheduler::get_current_command_buffer() noexcept {
        return m_frameManager.frames[m_frameManager.currentFrame].buffer;
    }
}
