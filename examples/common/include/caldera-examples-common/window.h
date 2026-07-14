#ifndef CALDERA_EXAMPLES_COMMON_WINDOW_H
#define CALDERA_EXAMPLES_COMMON_WINDOW_H

#include <memory>
#include <vector>

#include <caldera-examples-common/config.h>

struct GLFWwindow;

namespace caldera_examples_common
{
    struct Context
    {
    private:
        [[nodiscard]] static std::vector<const char*> get_extensions();

        [[nodiscard]] static bool init_glfw() noexcept;
        [[nodiscard]] static uint32_t get_version();
        [[nodiscard]] static vk::Instance init_instance(uint32_t version);

    public:
        [[nodiscard]] bool init();

        Context() noexcept;
        ~Context() noexcept;

        Context(Context &&) = delete;
        Context& operator=(Context &&) = delete;

        Context(Context const&) = delete;
        Context& operator=(Context const&) = delete;

        vk::Instance instance;
        uint32_t version;
    };

    struct Window
    {
        struct Deleter {
            void operator()(GLFWwindow* handle) const noexcept;
        };

    private:
        [[nodiscard]] static std::unique_ptr<GLFWwindow, Deleter> create_window();
        [[nodiscard]] static vk::SurfaceKHR create_surface(GLFWwindow* handle, vk::Instance instance);

    public:
        [[nodiscard]] bool init(Context const& context);
        void clear() noexcept;

        Window() noexcept;
        ~Window() noexcept;

        Window(Window &&) = delete;
        Window& operator=(Window &&) = delete;

        Window(Window const&) = delete;
        Window& operator=(Window const&) = delete;

        static void poll_events() noexcept;
        [[nodiscard]] bool closing() const noexcept;

    private:
        vk::Instance m_instance;

    public:
        std::unique_ptr<GLFWwindow, Deleter> window;
        vk::SurfaceKHR surface;
    };
}

#endif
