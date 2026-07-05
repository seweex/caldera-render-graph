
#include <scheduler.h>
#include <device.h>
#include <swapchain.h>
#include <spdlog/spdlog.h>

namespace caldera_example
{
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
        auto const semaphore = m_frameManager.frames[m_frameManager.currentFrame].imageAvailableSemaphore;
        auto const image = m_device.acquireNextImageKHR(swapchain.swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE);

        if (!image.has_value()) {
            spdlog::error("Failed to acquire a swapchain image: ", vk::to_string(image.result));
            return false;
        }

        m_currentImage = *image;
        return true;
    }

    bool Scheduler::end_frame(Swapchain const& swapchain)
    {
        auto const semaphore = m_frameManager.frames[m_frameManager.currentFrame].imageAvailableSemaphore;

        vk::PresentInfoKHR const presentInfo {
            1, &semaphore,
            1, &swapchain.swapchain, &m_currentImage
        };

        if (auto const result = m_queue.presentKHR(presentInfo);
            result < vk::Result::eSuccess)
        {
            spdlog::error("Failed to present an image: ", vk::to_string(result));
            return false;
        }

        m_frameManager.advance();
        return true;
    }

    vk::CommandBuffer Scheduler::get_current_command_buffer() noexcept {
        return m_frameManager.frames[m_frameManager.currentFrame].buffer;
    }
}
