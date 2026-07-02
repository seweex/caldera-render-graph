
#include <frame.h>
#include <device.h>

#include <spdlog/spdlog.h>

#include "swapchain.h"

namespace caldera_example
{
    /* * * * * * * * */
    /* Frame Context */
    /* * * * * * * * */

    vk::CommandPool FrameContext::create_pool(vk::Device const device, uint32_t const family)
    {
        vk::CommandPoolCreateInfo const createInfo
        {
            vk::CommandPoolCreateFlagBits::eTransient,
            family
        };

        auto const newPool = device.createCommandPool(createInfo);

        if (!newPool.has_value())
        {
            spdlog::error("Failed to create a command pool: {}", vk::to_string(newPool.result));
            return VK_NULL_HANDLE;
        }

        return *newPool;
    }

    vk::CommandBuffer FrameContext::allocate_buffer(vk::Device const device, vk::CommandPool const pool)
    {
        vk::CommandBufferAllocateInfo const allocateInfo
        {
            pool,
            vk::CommandBufferLevel::ePrimary,
            1
        };

        auto const newBuffer = device.allocateCommandBuffers(allocateInfo);

        if (!newBuffer.has_value())
        {
            spdlog::error("Failed to allocate a command buffer: {}", vk::to_string(newBuffer.result));
            return VK_NULL_HANDLE;
        }

        return std::move(newBuffer->front());
    }

    vk::Semaphore FrameContext::create_semaphore(vk::Device const device, bool const timeline)
    {
        vk::StructureChain const createInfo
        {
            vk::SemaphoreCreateInfo {
                vk::SemaphoreCreateFlags{}
            },
            vk::SemaphoreTypeCreateInfo {
                timeline ? vk::SemaphoreType::eTimeline : vk::SemaphoreType::eBinary, 0
            }
        };

        auto const newSemaphore = device.createSemaphore(createInfo.get<vk::SemaphoreCreateInfo>());

        if (!newSemaphore.has_value())
        {
            spdlog::error("Failed to create a semaphore: {}", vk::to_string(newSemaphore.result));
            return VK_NULL_HANDLE;
        }

        return *newSemaphore;
    }

    FrameContext::FrameContext() noexcept = default;

    FrameContext::~FrameContext() noexcept {
        clear();
    }

    FrameContext::FrameContext(FrameContext&& other) noexcept {
        *this = std::move(other);
    }

    FrameContext& FrameContext::operator=(FrameContext&& other) noexcept
    {
        if (this == &other) return *this;

        m_device = other.m_device;
        pool = other.pool;
        buffer = other.buffer;

        timelineSemaphore = other.timelineSemaphore;
        timelineValue = other.timelineValue;

        imageAvailableSemaphore = other.imageAvailableSemaphore;
        renderFinishedSemaphore = other.renderFinishedSemaphore;

        other.m_device = VK_NULL_HANDLE;
        other.pool = VK_NULL_HANDLE;
        other.buffer = VK_NULL_HANDLE;

        other.timelineSemaphore = VK_NULL_HANDLE;
        other.timelineValue = 0;

        other.imageAvailableSemaphore = VK_NULL_HANDLE;
        other.renderFinishedSemaphore = VK_NULL_HANDLE;

        return *this;
    }

    bool FrameContext::init(Device const& device)
    {
        m_device = device.device;
        timelineValue = 0;

        if (!(pool = create_pool(m_device, device.queueFamilyIndex)) ||
            !(buffer = allocate_buffer(m_device, pool)) ||
            !(timelineSemaphore = create_semaphore(m_device, true)) ||
            !(imageAvailableSemaphore = create_semaphore(m_device, false)) ||
            !(renderFinishedSemaphore = create_semaphore(m_device, false)))
        {
            clear();
            return false;
        }

        return true;
    }

    void FrameContext::clear() noexcept
    {
        if (m_device)
        {
            m_device.destroySemaphore(renderFinishedSemaphore);
            m_device.destroySemaphore(imageAvailableSemaphore);

            m_device.destroySemaphore(timelineSemaphore);
            m_device.destroyCommandPool();

            m_device.freeCommandBuffers(pool, 1, &buffer);
            m_device.destroyCommandPool(pool);

            renderFinishedSemaphore = VK_NULL_HANDLE;
            imageAvailableSemaphore = VK_NULL_HANDLE;
            timelineSemaphore = VK_NULL_HANDLE;

            buffer = VK_NULL_HANDLE;
            pool = VK_NULL_HANDLE;

            m_device = VK_NULL_HANDLE;
        }
    }

    /* * * * * * * * */
    /* Frame Manager */
    /* * * * * * * * */

    FrameManager::FrameManager() noexcept = default;
    FrameManager::~FrameManager() noexcept = default;

    bool FrameManager::init(Device const& device)
    {
        for (auto& frame : frames)
            if (!frame.init(device))
                return false;

        currentFrame = 0;

        return true;
    }

    void FrameManager::clear() noexcept
    {
        for (auto& frame : frames)
            frame.clear();
    }

    void FrameManager::advance() noexcept {
        currentFrame = (currentFrame + 1) % frames.size();
    }
}
