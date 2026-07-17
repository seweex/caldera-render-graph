
#include <caldera-render-graph/scheduler.h>

#include "caldera-render-graph/pass.h"

namespace
{
    [[nodiscard]] vk::CommandPool create_pool(
        vk::Device const device,
        uint32_t const family)
    {
        vk::CommandPoolCreateInfo const createInfo
            { vk::CommandPoolCreateFlagBits::eTransient, family };

        if (auto const pool = device.createCommandPool(createInfo);
            pool.has_value())
        {
            return *pool;
        }

        return VK_NULL_HANDLE;
    }

    [[nodiscard]] std::vector<vk::CommandBuffer> allocate_buffers(
        vk::Device const device,
        vk::CommandPool const pool)
    {
        uint32_t constexpr initial_buffer_count = 4;

        vk::CommandBufferAllocateInfo const allocateInfo
            { pool, vk::CommandBufferLevel::ePrimary, initial_buffer_count };

        if (auto buffers = device.allocateCommandBuffers(allocateInfo);
            buffers.result < vk::Result::eSuccess)
        {
            return std::move(*buffers);
        }

        return {};
    }

    [[nodiscard]] vk::Semaphore create_semaphore(vk::Device const device)
    {
        vk::StructureChain const createInfo
        {
            vk::SemaphoreCreateInfo{},
            vk::SemaphoreTypeCreateInfo{ vk::SemaphoreType::eTimeline, 0 }
        };

        if (auto const semaphore = device.createSemaphore(createInfo.get());
            semaphore.has_value())
        {
            return *semaphore;
        }

        return VK_NULL_HANDLE;
    }
}

namespace caldera::detail
{
    vk::CommandBuffer CommandBufferPool::allocate()
    {
        vk::CommandBuffer buffer;

        vk::CommandBufferAllocateInfo const allocateInfo
            { m_pool, vk::CommandBufferLevel::ePrimary, 1 };

        if (m_device.allocateCommandBuffers(&allocateInfo, &buffer) < vk::Result::eSuccess)
        {
            assert(0 && "failed to allocate a command buffer");
            std::abort();
        }

        return buffer;
    }

    CommandBufferPool::CommandBufferPool() noexcept :
        m_device            (VK_NULL_HANDLE),
        m_pool              (VK_NULL_HANDLE),
        m_activeBufferCount (0)
    {}

    CommandBufferPool::~CommandBufferPool() noexcept {
        clear();
    }

    CommandBufferPool::CommandBufferPool(CommandBufferPool&& other) noexcept :
        m_device            (other.m_device),
        m_pool              (other.m_pool),
        m_commandBuffers    (std::move(other.m_commandBuffers)),
        m_activeBufferCount (other.m_activeBufferCount)
    {
        other.m_device = VK_NULL_HANDLE;
        other.m_pool = VK_NULL_HANDLE;
        other.m_activeBufferCount = 0;
    }

    CommandBufferPool& CommandBufferPool::operator=(CommandBufferPool&& other) noexcept
    {
        if (&other == this) return *this;

        m_device = other.m_device;
        m_pool = other.m_pool;
        m_commandBuffers = std::move(other.m_commandBuffers);
        m_activeBufferCount = other.m_activeBufferCount;

        other.m_device = VK_NULL_HANDLE;
        other.m_pool = VK_NULL_HANDLE;
        other.m_activeBufferCount = 0;

        return *this;
    }

    bool CommandBufferPool::init(
        vk::Device const device,
        uint32_t const family)
    {
        if (*this) return true;

        m_device = device;
        m_activeBufferCount = 0;

        if (!(m_pool = create_pool(m_device, family)) ||
            (m_commandBuffers = allocate_buffers(m_device, m_pool)).empty())
        {
            clear();
            return false;
        }

        return true;
    }

    void CommandBufferPool::clear() noexcept
    {
        if (m_device)
        {
            m_device.destroyCommandPool(m_pool);

            m_device = VK_NULL_HANDLE;
            m_pool = VK_NULL_HANDLE;
            m_commandBuffers.clear();
            m_activeBufferCount = 0;
        }
    }

    void CommandBufferPool::reset()
    {
        if (m_device.resetCommandPool(m_pool) < vk::Result::eSuccess)
        {
            assert(0 && "failed to reset a command pool");
            std::abort();
        }
    }

    vk::CommandBuffer CommandBufferPool::request()
    {
        if (m_activeBufferCount == m_commandBuffers.size())
            m_commandBuffers.push_back(allocate());

        return m_commandBuffers[m_activeBufferCount++];
    }

    CommandBufferPool::operator bool() const noexcept {
        return m_device;
    }
}

namespace caldera
{
    void Scheduler::set_zero() noexcept
    {
        m_device = VK_NULL_HANDLE;
        m_familyIndexMapping.fill(0);
        m_queues.fill(VK_NULL_HANDLE);
        m_timelineSemaphore = VK_NULL_HANDLE;
        m_timelineValue = 0;
    }

    uint32_t Scheduler::map_family_index(QueueType const type) const noexcept {
        return m_familyIndexMapping[static_cast<int>(type) - 1];
    }

    Scheduler::Scheduler() noexcept {
        set_zero();
    }

    Scheduler::~Scheduler() noexcept {
        clear();
    }

    Scheduler::Scheduler(Scheduler&& other) noexcept :
        m_device                (other.m_device),
        m_familyIndexMapping    (other.m_familyIndexMapping),
        m_queues                (other.m_queues),
        m_pools                 (std::move(other.m_pools)),
        m_timelineSemaphore     (other.m_timelineSemaphore),
        m_timelineValue         (other.m_timelineValue),
        m_submissionCache       (std::move(other.m_submissionCache))
    {
        other.set_zero();
    }

    Scheduler& Scheduler::operator=(Scheduler&& other) noexcept
    {
        if (&other == this) return *this;

        m_device = other.m_device;
        m_familyIndexMapping = other.m_familyIndexMapping;
        m_queues = other.m_queues;
        m_pools = std::move(other.m_pools);
        m_timelineSemaphore = other.m_timelineSemaphore;
        m_timelineValue = other.m_timelineValue;
        m_submissionCache = std::move(other.m_submissionCache);

        other.set_zero();

        return *this;
    }

    bool Scheduler::init(
        vk::Device const device,
        QueueIndices const& indices)
    {
        if (m_device) return true;

        m_device = device;

        m_familyIndexMapping[static_cast<int>(QueueType::graphics) - 1] = indices.graphicsFamily;
        m_familyIndexMapping[static_cast<int>(QueueType::transfer) - 1] = indices.transferFamily;
        m_familyIndexMapping[static_cast<int>(QueueType::compute) - 1] = indices.computeFamily;

        uint32_t const graphicsIdx = map_family_index(QueueType::graphics);
        uint32_t const transferIdx = map_family_index(QueueType::transfer);
        uint32_t const computeIdx = map_family_index(QueueType::compute);

        m_queues[graphicsIdx] = m_device.getQueue(indices.graphicsFamily, indices.graphicsQueue);
        m_queues[transferIdx] = m_device.getQueue(indices.transferFamily, indices.transferQueue);
        m_queues[computeIdx] = m_device.getQueue(indices.computeFamily, indices.computeQueue);

        m_timelineValue = 1;
        m_submissionCache.reserve(5);

        if (!m_pools[graphicsIdx].init(m_device, indices.graphicsFamily) ||
            !m_pools[transferIdx].init(m_device, indices.transferFamily) ||
            !m_pools[computeIdx].init(m_device, indices.computeFamily) ||
            !(m_timelineSemaphore = create_semaphore(m_device)))
        {
            clear();
            return false;
        }

        return true;
    }

    void Scheduler::clear()
    {
        if (m_device)
        {
            m_device.destroySemaphore(m_timelineSemaphore);

            for (auto& pool : m_pools)
                pool.clear();

            set_zero();
        }
    }

    Scheduler::operator bool() const noexcept {
        return m_device;
    }

    void Scheduler::reset_buffers()
    {
        for (auto& pool : m_pools)
            if (pool)
                pool.reset();
    }

    vk::CommandBuffer Scheduler::get_buffer(QueueType const type)
    {
        auto const index = map_family_index(type);
        return m_pools[index].request();
    }

    detail::SubmissionID Scheduler::submit_buffer(
        QueueType const type,
        vk::CommandBuffer const buffer,
        vk::PipelineStageFlagBits2 const stages,
        std::span<const detail::SubmissionID> const deps)
    {
        auto const index = map_family_index(type);
        auto const queue = m_queues[index];
        auto& pool = m_pools[index];

        m_submissionCache.clear();
        m_submissionCache.reserve(deps.size());

        for (auto const dep : deps)
            m_submissionCache.emplace_back(
                m_timelineSemaphore, dep.timeline, dep.stages, 0);

        vk::SemaphoreSubmitInfo signalInfo;
        signalInfo.semaphore = m_timelineSemaphore;
        signalInfo.value = m_timelineValue;
        signalInfo.stageMask = stages;

        vk::CommandBufferSubmitInfo bufferInfo;
        bufferInfo.commandBuffer = buffer;

        vk::SubmitInfo2 submitInfo;
        submitInfo.setCommandBufferInfos(bufferInfo);
        submitInfo.setSignalSemaphoreInfos(signalInfo);
        submitInfo.setWaitSemaphoreInfos(m_submissionCache);

        if (queue.submit2(submitInfo) < vk::Result::eSuccess)
        {
            assert(0 && "failed to submit a command buffer");
            std::abort();
        }

        return detail::SubmissionID { m_timelineValue++, stages };
    }
}
