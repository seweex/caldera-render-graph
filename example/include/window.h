#ifndef CALDERA_EXAMPLE_WINDOW_H
#define CALDERA_EXAMPLE_WINDOW_H

#include <memory>
#include <optional>

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace caldera_example
{
    struct Context
    {
    private:
        [[nodiscard]] static std::vector<const char*> get_extensions();

        [[nodiscard]] static bool init_glfw() noexcept;
        [[nodiscard]] static vk::Instance init_instance();

    public:
        [[nodiscard]] bool init();

        Context() noexcept;
        ~Context() noexcept;

        Context(Context &&) = delete;
        Context& operator=(Context &&) = delete;

        Context(Context const&) = delete;
        Context& operator=(Context const&) = delete;

        vk::Instance instance{ VK_NULL_HANDLE };
    };

    struct Window
    {
        struct Deleter {
            void operator()(GLFWwindow* handle) const noexcept;
        };

        [[nodiscard]] bool init();

        Window() noexcept;
        ~Window() noexcept;

        Window(Window &&) = delete;
        Window& operator=(Window &&) = delete;

        Window(Window const&) = delete;
        Window& operator=(Window const&) = delete;

        static void poll_events() noexcept;
        [[nodiscard]] bool closing() const noexcept;

        std::unique_ptr<GLFWwindow, Deleter> window;
    };
}

#endif
