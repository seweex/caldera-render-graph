#ifndef CALDERA_EXAMPLE_DEVICE_H
#define CALDERA_EXAMPLE_DEVICE_H

#include <window.h>

#include <vulkan/vulkan.hpp>

namespace caldera_example
{
    struct Device
    {
    private:
        struct ChoiceResult
        {
            vk::PhysicalDevice device;
            uint32_t family;
        };

        [[nodiscard]] static uint32_t rate_device(vk::PhysicalDevice device, vk::SurfaceKHR surface);
        [[nodiscard]] static std::optional<uint32_t> get_family_index(vk::PhysicalDevice device, vk::SurfaceKHR surface);
        [[nodiscard]] static std::optional<ChoiceResult> choose_device(Context const& context, Window const& window);

        [[nodiscard]] static vk::Device create_device(ChoiceResult choice);

    public:
        [[nodiscard]] bool init(Context const& context, Window const& window);

        Device(Device &&) = delete;
        Device& operator=(Device &&) = delete;

        Device(Device const&) = delete;
        Device& operator=(Device const&) = delete;

        Device() noexcept;
        ~Device() noexcept;

        vk::PhysicalDevice physicalDevice{ VK_NULL_HANDLE };
        vk::Device device{ VK_NULL_HANDLE };

        uint32_t queueFamilyIndex;
    };
}

#endif
