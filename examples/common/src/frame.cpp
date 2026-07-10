
#include <frame.h>
#include <../../common/include/device.h>

#include <spdlog/spdlog.h>

namespace
{
    [[nodiscard]] vk::Semaphore create_binary_semaphore(vk::Device const device)
    {
        vk::SemaphoreCreateInfo constexpr createInfo {
            vk::SemaphoreCreateFlags{}
        };

        auto const newSemaphore = device.createSemaphore(createInfo);

        if (!newSemaphore.has_value()) {
            spdlog::error("Failed to create a semaphore: {}", vk::to_string(newSemaphore.result));
            return VK_NULL_HANDLE;
        }

        return *newSemaphore;
    }

    [[nodiscard]] vk::Fence create_fence(vk::Device const device)
    {
        vk::FenceCreateInfo constexpr createInfo
            { vk::FenceCreateFlagBits::eSignaled };

        auto const newFence = device.createFence(createInfo);

        if (!newFence.has_value()) {
            spdlog::error("Failed to create a fence: {}", vk::to_string(newFence.result));
            return VK_NULL_HANDLE;
        }

        return *newFence;
    }

    [[nodiscard]] vk::CommandPool create_pool(vk::Device const device, uint32_t const family)
    {
        vk::CommandPoolCreateInfo const createInfo {
            vk::CommandPoolCreateFlagBits::eTransient,
            family
        };

        auto const newPool = device.createCommandPool(createInfo);

        if (!newPool.has_value()) {
            spdlog::error("Failed to create a command pool: {}", vk::to_string(newPool.result));
            return VK_NULL_HANDLE;
        }

        return *newPool;
    }

    [[nodiscard]] std::vector<vk::CommandBuffer> allocate_buffers(
        vk::Device const device,
        vk::CommandPool const pool,
        uint32_t const count)
    {
        vk::CommandBufferAllocateInfo const allocateInfo {
            pool, vk::CommandBufferLevel::ePrimary, count
        };

        auto newBuffers = device.allocateCommandBuffers(allocateInfo);

        if (!newBuffers.has_value()) {
            spdlog::error("Failed to allocate a command buffer: {}", vk::to_string(newBuffers.result));
            return {};
        }

        return std::move(*newBuffers);
    }
}

namespace caldera_example
{
    FrameResources::FrameResources() noexcept = default;
    FrameResources::~FrameResources() noexcept = default;

    bool FrameResources::init(
        vk::Device const device,
        uint32_t const family,
        uint32_t const bufferCount)
    {
        if (!(pool = create_pool(device, family)) ||
            (buffers = allocate_buffers(device, pool, bufferCount)).empty() ||
            !(imageAvailableSemaphore = create_binary_semaphore(device)) ||
            !(renderFinishedSemaphore = create_binary_semaphore(device)) ||
            !(imagePresentedFence = create_fence(device)))
        {
            clear(device);
            return false;
        }

        activeBufferIndex = 0;
        previousSubmissionTicket = 0;

        return true;
    }

    void FrameResources::clear(vk::Device const device) noexcept
    {
        device.destroyFence(imagePresentedFence);
        device.destroySemaphore(renderFinishedSemaphore);
        device.destroySemaphore(imageAvailableSemaphore);
        device.destroyCommandPool(pool);

        pool = VK_NULL_HANDLE;
        buffers.clear();

        imageAvailableSemaphore = VK_NULL_HANDLE;
        renderFinishedSemaphore = VK_NULL_HANDLE;
        imagePresentedFence = VK_NULL_HANDLE;
    }
}
