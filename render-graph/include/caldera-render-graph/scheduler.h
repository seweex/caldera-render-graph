#ifndef CALDERA_RENDER_GRAPH_SCHEDULER_H
#define CALDERA_RENDER_GRAPH_SCHEDULER_H

#include <caldera-render-graph/config.h>

namespace caldera::detail
{
    class CommandBufferPool
    {
        [[nodiscard]] vk::CommandBuffer allocate();

    public:
        CommandBufferPool() noexcept;
        ~CommandBufferPool() noexcept;

        CommandBufferPool(CommandBufferPool&&) noexcept;
        CommandBufferPool& operator=(CommandBufferPool&&) noexcept;

        CommandBufferPool(CommandBufferPool const&) = delete;
        CommandBufferPool& operator=(CommandBufferPool const&) = delete;

        [[nodiscard]] bool init(vk::Device device, uint32_t family);
        void clear() noexcept;

        void reset();
        [[nodiscard]] vk::CommandBuffer request();

        [[nodiscard]] explicit operator bool() const noexcept;

    private:
        vk::Device m_device;
        vk::CommandPool m_pool;

        std::vector<vk::CommandBuffer> m_commandBuffers;
        uint32_t m_activeBufferCount;
    };
}

namespace caldera
{
    struct QueueIndices
    {
        uint32_t graphicsFamily;
        uint32_t transferFamily;
        uint32_t computeFamily;

        uint32_t graphicsQueue;
        uint32_t transferQueue;
        uint32_t computeQueue;
    };

    class Scheduler
    {
        void set_zero() noexcept;
        [[nodiscard]] uint32_t map_family_index(QueueType type) const noexcept;

    public:
        Scheduler() noexcept;
        ~Scheduler() noexcept;

        Scheduler(Scheduler&&) noexcept;
        Scheduler& operator=(Scheduler&&) noexcept;

        Scheduler(Scheduler const&) = delete;
        Scheduler& operator=(Scheduler const&) = delete;

        [[nodiscard]] bool init(vk::Device device, QueueIndices const& indices);
        void clear();

        [[nodiscard]] explicit operator bool() const noexcept;

    private:
        friend class ::caldera::RenderGraph;

        void reset_buffers();
        [[nodiscard]] vk::CommandBuffer get_buffer(QueueType type);

        [[nodiscard]] detail::SubmissionID submit_buffer(
            QueueType type,
            vk::CommandBuffer buffer,
            vk::PipelineStageFlagBits2 stages,
            std::span<detail::SubmissionID const> deps);

        /* * * * */

        vk::Device m_device;

        std::array<uint32_t, 3> m_familyIndexMapping;
        std::array<vk::Queue, 3> m_queues;
        std::array<detail::CommandBufferPool, 3> m_pools;

        vk::Semaphore m_timelineSemaphore;
        uint64_t m_timelineValue;

        std::vector<vk::SemaphoreSubmitInfo> m_submissionCache;
    };
}

namespace caldera::detail
{
    struct SubmissionID
    {
        uint64_t timeline;
        vk::PipelineStageFlagBits2 stages;
    };
}

#endif
