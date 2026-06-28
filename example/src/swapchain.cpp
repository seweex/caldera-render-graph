
#include <swapchain.h>

#include <spdlog/spdlog.h>

#include "device.h"

namespace caldera_example
{
    /* * * * * * */
    /* Swapchain */
    /* * * * * * */

    std::optional<Swapchain::Configuration> Swapchain::choose_configuration(
        vk::PhysicalDevice const device,
        vk::SurfaceKHR const surface)
    {
        Configuration result;
        vk::SurfaceFormatKHR constexpr target_format{ vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear };

        /* Format & Color space */

        auto const surfaceFormats = device.getSurfaceFormatsKHR(surface);

        if (!surfaceFormats.has_value())
        {
            spdlog::error("Failed to get surface formats: {}",
                vk::to_string(surfaceFormats.result));

            return std::nullopt;
        }

        bool formatFound = false;

        for (auto const& surfaceFormat : *surfaceFormats)
            if (surfaceFormat == target_format)
            {
                formatFound = true;
                result.colorSpace = target_format.colorSpace;
                result.format = surfaceFormat.format;
                break;
            }

        if (!formatFound) {
            spdlog::error("No suitable surface format was found");
            return std::nullopt;
        }

        /* Extent */

        auto const surfaceCapabilities = device.getSurfaceCapabilitiesKHR(surface);

        if (!surfaceCapabilities.has_value())
        {
            spdlog::error("Failed to get surface capabilities: {}",
                vk::to_string(surfaceFormats.result));

            return std::nullopt;
        }

        if (Swapchain::image_count > surfaceCapabilities->maxImageCount ||
            Swapchain::image_count < surfaceCapabilities->minImageCount)
        {
            spdlog::error("Image count ({}) is not available for the surface", Swapchain::image_count);
            return std::nullopt;
        }

        if ((Swapchain::image_usage & surfaceCapabilities->supportedUsageFlags) != Swapchain::image_usage)
        {
            spdlog::error("Required usages of surface aren't supported. Supported: ");
            return std::nullopt;
        }

        result.extent = surfaceCapabilities->currentExtent;

        return result;
    }

    vk::SwapchainKHR Swapchain::create_swapchain(
        vk::Device const device,
        uint32_t const family,
        vk::SurfaceKHR const surface,
        Configuration const config)
    {
        vk::SwapchainCreateInfoKHR const createInfo
        {
            vk::SwapchainCreateFlagsKHR{},
            surface,
            image_count,
            config.format,
            config.colorSpace,
            config.extent,
            1,
            Swapchain::image_usage,
            vk::SharingMode::eExclusive,
            1, &family,
            vk::SurfaceTransformFlagBitsKHR::eIdentity,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            vk::PresentModeKHR::eFifo,
            vk::True
        };

        auto const swapchain = device.createSwapchainKHR(createInfo);

        if (!swapchain.has_value())
        {
            spdlog::error("Failed to create swapchain: {}", vk::to_string(swapchain.result));
            return VK_NULL_HANDLE;
        }

        return *swapchain;
    }

    bool Swapchain::init(Device const& device, Window const& window)
    {
        auto const config = choose_configuration(device.physicalDevice, window.surface);

        if (!config.has_value())
            return false;

        auto const newSwapchain = create_swapchain(
            device.device, device.queueFamilyIndex, window.surface, *config);

        if (!newSwapchain)
            return false;

        auto newImages = device.device.getSwapchainImagesKHR(newSwapchain);

        if (!newImages.has_value())
        {
            spdlog::error("Failed to get swapchain images: {}", vk::to_string(newImages.result));
            return false;
        }

        swapchain = newSwapchain;
        images = std::move(newImages.value);
        m_device = device.device;

        return true;
    }

    Swapchain::Swapchain() noexcept = default;

    Swapchain::~Swapchain() noexcept {
        if (m_device)
            m_device.destroySwapchainKHR(swapchain);
    }
}
