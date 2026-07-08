
#include <swapchain.h>
#include <device.h>
#include <window.h>

#include <spdlog/spdlog.h>

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
        vk::SurfaceFormatKHR constexpr target_format{ target_image_format, vk::ColorSpaceKHR::eSrgbNonlinear };

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

    std::vector<vk::ImageView> Swapchain::create_views(
        vk::Device const device,
        std::vector<vk::Image> const& images,
        vk::Format const format)
    {
        std::vector<vk::ImageView> result;
        result.reserve(images.size());

        for (auto const image : images)
        {
            vk::ImageViewCreateInfo const createInfo
            {
                vk::ImageViewCreateFlags{},
                image,
                vk::ImageViewType::e2D,
                format,
                vk::ComponentMapping{},
                vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
            };

            auto const newView = device.createImageView(createInfo);

            if (!newView.has_value())
            {
                spdlog::error("Failed to create image view: {}", vk::to_string(newView.result));

                for (auto const view : result)
                    device.destroyImageView(view);

                return {};
            }

            result.push_back(*newView);
        }

        return result;
    }

    bool Swapchain::init(Device const& device, Window const& window)
    {
        auto const configChoice = choose_configuration(device.physicalDevice, window.surface);

        m_device = device.device;
        configuration = *configChoice;

        if (!configChoice.has_value())
            return false;

        if (!(swapchain = create_swapchain(device.device, device.queueFamilyIndex, window.surface, *configChoice)))
        {
            clear();
            return false;
        }

        if (auto newImages = device.device.getSwapchainImagesKHR(swapchain);
            !newImages.has_value())
        {
            clear();
            spdlog::error("Failed to get swapchain images: {}", vk::to_string(newImages.result));
            return false;
        }
        else {
            images = std::move(newImages.value);
        }

        if ((imageViews = create_views(device.device, images, configChoice->format)).empty())
        {
            clear();
            return false;
        }

        return true;
    }

    void Swapchain::clear() noexcept
    {
        if (m_device)
        {
            for (auto const view : imageViews)
                m_device.destroyImageView(view);

            m_device.destroySwapchainKHR(swapchain);

            imageViews.clear();
            images.clear();

            swapchain = VK_NULL_HANDLE;
            m_device = VK_NULL_HANDLE;
        }
    }

    Swapchain::Swapchain() noexcept = default;

    Swapchain::~Swapchain() noexcept {
        clear();
    }
}
