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
    private:
        static constexpr uint32_t image_count = 3;
        static constexpr vk::ImageUsageFlags image_usage = vk::ImageUsageFlagBits::eColorAttachment;

        struct Configuration
        {
            vk::Format format;
            vk::ColorSpaceKHR colorSpace;
            vk::Extent2D extent;
        };

        [[nodiscard]] static std::optional<Configuration> choose_configuration(
            vk::PhysicalDevice device, vk::SurfaceKHR surface);

        [[nodiscard]] static vk::SwapchainKHR create_swapchain(
            vk::Device device, uint32_t family, vk::SurfaceKHR surface, Configuration config);

    public:
        [[nodiscard]] bool init(Device const& device, Window const& window);

        Swapchain() noexcept;
        ~Swapchain() noexcept;

        Swapchain(Swapchain &&) = delete;
        Swapchain& operator=(Swapchain &&) = delete;

        Swapchain(Swapchain const&) = delete;
        Swapchain& operator=(Swapchain const&) = delete;

        vk::SwapchainKHR swapchain{ VK_NULL_HANDLE };
        std::vector<vk::Image> images;

    private:
        vk::Device m_device{ VK_NULL_HANDLE };
    };
}

#endif
