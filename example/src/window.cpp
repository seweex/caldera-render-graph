
#include <window.h>

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace caldera_example
{
    /* * * * * */
    /* Context */
    /* * * * * */

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

    vk::Instance Context::init_instance()
    {
        constexpr auto layer = "VK_LAYER_KHRONOS_validation";

        auto const extensions = get_extensions();

        if (extensions.empty()) {
            spdlog::error("Failed to get vulkan extensions");
            return VK_NULL_HANDLE;
        }

        auto const instanceVersion = vk::enumerateInstanceVersion();

        if (!instanceVersion.has_value()) {
            spdlog::error("Failed to enumerate vulkan instance version");
            return VK_NULL_HANDLE;
        }

        vk::ApplicationInfo const applicationInfo
        {
            "Caldera Example",
            0,
            "No engine",
            0,
            *instanceVersion
        };

        vk::InstanceCreateInfo const createInfo
        {
            vk::InstanceCreateFlags{},
            &applicationInfo,
            1, &layer,
            static_cast<uint32_t>(extensions.size()), extensions.data()
        };

        auto const instance = vk::createInstance(createInfo);
        return instance.has_value() ? *instance : VK_NULL_HANDLE;
    }

    bool Context::init()
    {
        if (!init_glfw())
            return false;

        if (auto const newInstance = init_instance(); !newInstance)
            return false;
        else
            instance = newInstance;

        return true;
    }

    Context::Context() noexcept = default;

    Context::~Context() noexcept
    {
        instance.destroy();
        glfwTerminate();
    }

    /* * * *  */
    /* Window */
    /* * * *  */

    void Window::Deleter::operator()(GLFWwindow* const handle) const noexcept
    {
        glfwDestroyWindow(handle);
    }

    bool Window::init()
    {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        auto const handle = glfwCreateWindow(1024, 576, "Caldera Example", nullptr, nullptr);

        if (!handle)
            return false;

        window = std::unique_ptr<GLFWwindow, Deleter>{ handle, Deleter{} };
        return true;
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

    Window::Window() noexcept = default;
    Window::~Window() noexcept = default;
}
