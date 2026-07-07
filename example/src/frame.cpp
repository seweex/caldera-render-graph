
#include <frame.h>
#include <device.h>

#include <spdlog/spdlog.h>

namespace {
    vk::Semaphore create_semaphore(vk::Device const device, bool const timeline)
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
}

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

    std::vector<vk::CommandBuffer> FrameContext::allocate_buffers(vk::Device const device, vk::CommandPool const pool)
    {
        vk::CommandBufferAllocateInfo const allocateInfo
        {
            pool,
            vk::CommandBufferLevel::ePrimary,
            1
        };

        auto newBuffers = device.allocateCommandBuffers(allocateInfo);

        if (!newBuffers.has_value())
        {
            spdlog::error("Failed to allocate a command buffer: {}", vk::to_string(newBuffers.result));
            return {};
        }

        return std::move(*newBuffers);
    }

    FrameContext::FrameContext() noexcept = default;
    FrameContext::~FrameContext() noexcept = default;

    bool FrameContext::init(Device const& device)
    {
        if (!(pool = create_pool(device.device, device.queueFamilyIndex)) ||
            (buffers = allocate_buffers(device.device, pool)).empty() ||
            !(imageAvailableSemaphore = create_semaphore(device.device, false)) ||
            !(renderFinishedSemaphore = create_semaphore(device.device, false)))
        {
            clear(device.device);
            return false;
        }

        activeBuffer = 0;
        previousSubmissionTicket = 0;

        return true;
    }

    void FrameContext::clear(vk::Device const device) noexcept
    {
        device.destroySemaphore(renderFinishedSemaphore);
        device.destroySemaphore(imageAvailableSemaphore);

        device.freeCommandBuffers(pool, buffers);
        device.destroyCommandPool(pool);

        renderFinishedSemaphore = VK_NULL_HANDLE;
        imageAvailableSemaphore = VK_NULL_HANDLE;

        buffers.clear();
        pool = VK_NULL_HANDLE;
    }

    /* * * * * * * * */
    /* Frame Manager */
    /* * * * * * * * */

    FrameManager::FrameManager() noexcept = default;

    FrameManager::~FrameManager() noexcept {
        clear();
    }

    bool FrameManager::init(Device const& device)
    {
        m_device = device.device;

        if (!(timelineSemaphore = create_semaphore(device.device, true)))
            return false;

        for (auto& frame : frames)
            if (!frame.init(device)) {
                clear();
                return false;
            }

        timelineValue = 0;
        currentFrame = 0;

        return true;
    }

    void FrameManager::clear() noexcept
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

    bool FrameManager::reset_current_pool()
    {
        if (auto const result = m_device.resetCommandPool(frames[currentFrame].pool);
            result < vk::Result::eSuccess)
            {
                spdlog::error("Failed to reset command pool: {}", vk::to_string(result));
                return false;
            }

        frames[currentFrame].activeBuffer = 0;
        return true;
    }

    void FrameManager::advance() noexcept
    {
        frames[currentFrame].previousSubmissionTicket = timelineValue;
        currentFrame = (currentFrame + 1) % frames.size();
    }
}
