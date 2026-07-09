#ifndef CALDERA_GRAPH_H
#define CALDERA_GRAPH_H

#include <config.h>
#include <resource.h>

namespace caldera
{
    /**
     * @brief The Render Graph class
     * @note For detailed usage see the example/src/main.cpp file
     */
    class RenderGraph
    {
        /// @internal
        [[nodiscard]] uint32_t choose_family(QueueType type) const noexcept;

        /// @internal
        [[nodiscard]] detail::TextureVulkanState gen_low_level_state(detail::TextureState state) const;

        /// @internal
        [[nodiscard]] detail::BufferVulkanState gen_low_level_state(detail::BufferState state) const;

    public:
        /**
         * @param device Vulkan device
         * @param graphicsFamily Index of graphic queue family
         * @param transferFamily Index of transfer queue family
         * @param computeFamily Index of compute queue family
         *
         * @note Queue family indices can have the same value
         */
        RenderGraph(
            vk::Device device,
            uint32_t graphicsFamily,
            uint32_t transferFamily,
            uint32_t computeFamily);

        RenderGraph(RenderGraph&&) noexcept = default;
        RenderGraph& operator=(RenderGraph&&) noexcept = default;

        RenderGraph(RenderGraph const&) = delete;
        RenderGraph& operator=(RenderGraph const&) = delete;

        /**
         * @brief Declares a virtual texture resource and adds it to the registry
         *
         * @param aspects Vulkan aspect flags of the target texture
         * @return ID of the declared texture
         */
        [[nodiscard]] TextureID declare_texture(vk::ImageAspectFlags aspects);

        /**
         * @brief Declares a virtual buffer resource and adds it to the registry
         *
         * @param size Size of the target buffer
         * @return ID of the declared buffer
         */
        [[nodiscard]] BufferID declare_buffer(vk::DeviceSize size);

        /**
         * @brief Associates the virtual ID to a Vulkan image
         *
         * @param id Virtual texture ID given by this Render Graph
         * @param image Vulkan image handle
         * @param view Vulkan image view handle of the image passed before
         *
         * @note image must have aspect flags specified in the declare_texture method
         * for this ID
         *
         * @note This method can be called before and after compilation. No recompilation needed
         */
        void associate(TextureID id, vk::Image image, vk::ImageView view);

        /**
         * @brief Associates the virtual ID to a Vulkan buffer
         *
         * @param id Virtual buffer ID given by this Render Graph
         * @param buffer Vulkan buffer handle
         *
         * @note buffer must be the same size specified in the declare_texture method
         * for this ID
         *
         * @note This method can be called before and after compilation. No recompilation needed
         */
        void associate(BufferID id, vk::Buffer buffer);

        /**
         * @brief Adds a Pass object to the execution
         * @param pass Described Pass node object
         */
        void push_pass(PassNode pass);

        /**
         * @brief Compiles the Render Graph and bakes the virtual resource transitions
         * @note This method must be called only once before the first execute() call
         */
        void compile();

        /**
         * @brief Traversing the compiled Render Graph
         * @param cmd Begun Vulkan command buffer for writing pass commands
         *
         * @note Render Graph doesn't begin or end the command buffers. You need to
         * begin it before calling this method.
         *
         * @note Before calling this method, you need to compile the graph
         */
        void execute(vk::CommandBuffer cmd);

    private:
        vk::Device m_device;

        uint32_t m_graphicsFamily;
        uint32_t m_transferFamily;
        uint32_t m_computeFamily;

        std::vector<PassNode> m_passes;

        std::vector<detail::TextureDescription> m_textures;
        std::vector<detail::BufferDescription> m_buffers;
    };
}

#endif
