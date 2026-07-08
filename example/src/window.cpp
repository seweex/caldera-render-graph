
#include <window.h>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <spdlog/spdlog.h>

namespace caldera_example
{
    /* * * * * */
    /* Context */
    /* * * * * */

    Context::Context() noexcept = default;

    std::vector<const char*> Context::get_extensions()
    {
        uint32_t extensionCount;
        auto const* extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        if (!extensions)
            return {};

        std::vector<const char*> result;
        result.reserve(extensionCount + 1);

        result.insert(result.end(), extensions, extensions + extensionCount);
        result.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        result.push_back(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
        result.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

        return result;
    }

    bool Context::init_glfw() noexcept
    {
        glfwSetErrorCallback([](int error, const char* description)
            { spdlog::error("GLFW error #{}: {}", error, description); });

        if (glfwInit() != GLFW_TRUE)
            return false;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        return true;
    }

    uint32_t Context::get_version()
    {
        auto const instanceVersion = vk::enumerateInstanceVersion();

        if (!instanceVersion.has_value()) {
            spdlog::error("Failed to enumerate vulkan instance version");
            return 0;
        }

        if (*instanceVersion < VK_API_VERSION_1_4) {
            spdlog::error("Vulkan version is below the required version");
            return 0;
        }

        return *instanceVersion;
    }

    vk::Instance Context::init_instance(uint32_t const version)
    {
        constexpr auto layer = "VK_LAYER_KHRONOS_validation";

        auto const extensions = get_extensions();

        if (extensions.empty()) {
            spdlog::error("Failed to get vulkan extensions");
            return VK_NULL_HANDLE;
        }

        vk::ApplicationInfo const applicationInfo
        {
            "Caldera Example",
            0,
            "No engine",
            0,
            version
        };

        vk::InstanceCreateInfo const createInfo
        {
            vk::InstanceCreateFlags{},
            &applicationInfo,
            1, &layer,
            static_cast<uint32_t>(extensions.size()), extensions.data()
        };

        auto const instance = vk::createInstance(createInfo);

        if (!instance.has_value()) {
            spdlog::error("Failed to create an instance: {}", vk::to_string(instance.result));
            return VK_NULL_HANDLE;
        }

        return *instance;
    }

    bool Context::init()
    {
        version = get_version();

        if (!init_glfw() ||
            version == 0 ||
            !(instance = init_instance(version)))
        {
            return false;
        }

        return true;
    }

    Context::~Context() noexcept
    {
        instance.destroy();
        glfwTerminate();
    }

    /* * * *  */
    /* Window */
    /* * * *  */

    Window::Window() noexcept = default;

    std::unique_ptr<GLFWwindow, Window::Deleter> Window::create_window()
    {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        auto const handle = glfwCreateWindow(1024, 576, "Caldera Example", nullptr, nullptr);

        return { handle, Deleter{} };
    }

    vk::SurfaceKHR Window::create_surface(GLFWwindow* const handle, vk::Instance const instance)
    {
        VkSurfaceKHR surface;
        auto const result = glfwCreateWindowSurface(instance, handle, nullptr, &surface);

        if (result < 0)
        {
            spdlog::error("Failed to create window surface: {}",
                vk::to_string(static_cast<vk::Result>(result)));

            return VK_NULL_HANDLE;
        }

        return surface;
    }

    void Window::Deleter::operator()(GLFWwindow* const handle) const noexcept
    {
        glfwDestroyWindow(handle);
    }

    bool Window::init(Context const& context)
    {
        auto newWindow = create_window();

        if (!newWindow)
            return false;

        auto const newSurface = create_surface(newWindow.get(), context.instance);

        if (!newSurface)
            return false;

        window = std::move(newWindow);
        surface = newSurface;
        m_instance = context.instance;

        return true;
    }

    void Window::clear() noexcept
    {
        if (m_instance)
        {
            m_instance.destroySurfaceKHR(surface);
            window = nullptr;

            surface = VK_NULL_HANDLE;
            m_instance = VK_NULL_HANDLE;
        }
    }

    void Window::poll_events() noexcept
    {
        glfwPollEvents();
    }

    bool Window::closing() const noexcept
    {
        assert(window);
        return glfwWindowShouldClose(window.get());
    }

    Window::~Window() noexcept {
        if (surface)
            m_instance.destroySurfaceKHR(surface);
    }
}
