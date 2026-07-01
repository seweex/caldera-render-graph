
#include <device.h>
#include <window.h>

#include <cmath>
#include <vector>
#include <array>
#include <spdlog/spdlog.h>

namespace caldera_example
{
    /* * * *  */
    /* Device */
    /* * * *  */

    uint32_t Device::rate_device(vk::PhysicalDevice const device, vk::SurfaceKHR const surface)
    {
        auto const presentModes = device.getSurfacePresentModesKHR(surface);
        auto const surfaceFormats = device.getSurfaceFormatsKHR(surface);

        if (!presentModes.has_value()) {
            spdlog::warn("Failed to get device present modes: {}", vk::to_string(presentModes.result));
            return 0;
        }

        if (!surfaceFormats.has_value()) {
            spdlog::warn("Failed to get device surface formats: {}", vk::to_string(surfaceFormats.result));
            return 0;
        }

        if (presentModes->empty() || surfaceFormats->empty())
            return 0;

        uint32_t accumulator = 0;

        switch (auto const commonProperties = device.getProperties();
                commonProperties.deviceType)
        {
        case vk::PhysicalDeviceType::eDiscreteGpu:
            accumulator += 1'000'000;
            break;

        case vk::PhysicalDeviceType::eVirtualGpu:
            accumulator += 400'000;
            break;

        case vk::PhysicalDeviceType::eIntegratedGpu:
            accumulator += 200'000;
            break;

        case vk::PhysicalDeviceType::eCpu:
            accumulator += 70'000;
            break;

        default:
            accumulator += 10'000;
            break;
        }

        auto const memoryProperties = device.getMemoryProperties();
        vk::DeviceSize totalLocalBytes = 0;

        for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i)
            if (auto const heapProperties = memoryProperties.memoryHeaps[i];
                    heapProperties.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
                totalLocalBytes += heapProperties.size;

        auto const sizeGB = static_cast<double>(totalLocalBytes) / 1024.0 / 1024.0 / 1024.0;
        accumulator += static_cast<uint32_t>(std::max(0.0, 10'000.0 * std::log2(sizeGB)));

        return accumulator;
    }

    std::optional<uint32_t> Device::get_family_index(vk::PhysicalDevice const device, vk::SurfaceKHR const surface)
    {
        constexpr auto required_flags = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer;
        auto const queueProperties = device.getQueueFamilyProperties();

        for (uint32_t i = 0; i < queueProperties.size(); ++i)
        {
            auto const surfaceSupport = device.getSurfaceSupportKHR(i, surface);

            if (static_cast<int>(surfaceSupport.result) < 0)
            {
                spdlog::warn("Vulkan error {} during checking surface support",
                    vk::to_string(surfaceSupport.result));

                continue;
            }

            if (required_flags == (queueProperties[i].queueFlags & required_flags) && surfaceSupport.value == vk::True)
                return i;
        }

        return std::nullopt;
    }

    std::optional<Device::ChoiceResult> Device::choose_device(Context const& context, Window const& window)
    {
        auto const devices = context.instance.enumeratePhysicalDevices();

        if (!devices.has_value()) {
            spdlog::error("Failed to enumerate physical devices: {}", vk::to_string(devices.result));
            return std::nullopt;
        }

        vk::PhysicalDevice bestDevice = VK_NULL_HANDLE;
        uint32_t bestDeviceFamily = 0;

        uint32_t bestRating = 0;

        for (auto const device : *devices)
        {
            auto const family = get_family_index(device, window.surface);

            if (!family.has_value())
                continue;

            if (auto const rating = rate_device(device, window.surface);
                rating > bestRating)
            {
                bestRating = rating;
                bestDeviceFamily = *family;
                bestDevice = device;
            }
        }

        if (!bestDevice) {
            spdlog::error("No suitable device was found");
            return std::nullopt;
        }

        return ChoiceResult{ bestDevice, bestDeviceFamily };
    }

    vk::Device Device::create_device(ChoiceResult const choice)
    {
        auto constexpr priority = 1.0f;

        vk::DeviceQueueCreateInfo const queueCreateInfo
        {
            vk::DeviceQueueCreateFlags{},
            choice.family,
            1, &priority
        };

        vk::PhysicalDeviceVulkan12Features deviceFeatures12;
        deviceFeatures12.timelineSemaphore = vk::True;

        vk::PhysicalDeviceVulkan13Features deviceFeatures13;
        deviceFeatures13.dynamicRendering = vk::True;
        deviceFeatures13.synchronization2 = vk::True;

        vk::PhysicalDeviceVulkan14Features deviceFeatures14;
        deviceFeatures14.pushDescriptor = vk::True;

        constexpr std::array deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        vk::StructureChain const deviceCreateInfo
        {
            vk::DeviceCreateInfo
            {
                vk::DeviceCreateFlags{},
                1, &queueCreateInfo,
                0, nullptr,
                deviceExtensions.size(), deviceExtensions.data()
            },
            deviceFeatures12,
            deviceFeatures13,
            deviceFeatures14
        };

        auto const newDevice = choice.device.createDevice(deviceCreateInfo.get<vk::DeviceCreateInfo>());

        if (!newDevice.has_value()) {
            spdlog::error("Failed to create vulkan device: {}", vk::to_string(newDevice.result));
            return VK_NULL_HANDLE;
        }

        return *newDevice;
    }

    bool Device::init(Context const& context, Window const& window)
    {
        auto const newPhysicalDevice = choose_device(context, window);

        if (!newPhysicalDevice)
            return false;

        auto const newDevice = create_device(*newPhysicalDevice);

        if (!newDevice)
            return false;

        physicalDevice = newPhysicalDevice->device;
        device = newDevice;
        queueFamilyIndex = newPhysicalDevice->family;

        return true;
    }

    Device::Device() noexcept = default;

    Device::~Device() noexcept {
        device.destroy();
    }
}
