#ifndef CALDERA_EXAMPLE_SWAPCHAIN_H
#define CALDERA_EXAMPLE_SWAPCHAIN_H

#include <array>

#include <vulkan/vulkan.hpp>

namespace caldera_example
{
    struct Device;
    struct Window;

    struct Swapchain
    {
        struct Configuration
        {
            vk::Format format;
            vk::ColorSpaceKHR colorSpace;
            vk::Extent2D extent;
        };

    private:
        static constexpr uint32_t image_count = 3;
        static constexpr vk::ImageUsageFlags image_usage = vk::ImageUsageFlagBits::eColorAttachment;

        [[nodiscard]] static std::optional<Configuration> choose_configuration(
            vk::PhysicalDevice device, vk::SurfaceKHR surface);

        [[nodiscard]] static vk::SwapchainKHR create_swapchain(
            vk::Device device, uint32_t family, vk::SurfaceKHR surface, Configuration config);

        [[nodiscard]] static std::vector<vk::ImageView> create_views(
            vk::Device device, std::vector<vk::Image> const& images, vk::Format format);

    public:
        [[nodiscard]] bool init(Device const& device, Window const& window);
        void clear() noexcept;

        Swapchain() noexcept;
        ~Swapchain() noexcept;

        Swapchain(Swapchain &&) = delete;
        Swapchain& operator=(Swapchain &&) = delete;

        Swapchain(Swapchain const&) = delete;
        Swapchain& operator=(Swapchain const&) = delete;

    private:
        vk::Device m_device;

    public:
        vk::SwapchainKHR swapchain;
        Configuration configuration;

        std::vector<vk::Image> images;
        std::vector<vk::ImageView> imageViews;
    };
}

#endif
