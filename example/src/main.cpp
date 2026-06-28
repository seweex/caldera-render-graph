
#include <iostream>

#include <window.h>
#include <device.h>
#include <swapchain.h>

int main()
{
    caldera_example::Context ctx;
    caldera_example::Window wnd;
    caldera_example::Device dvc;
    caldera_example::Swapchain swp;

    if (!ctx.init() || !wnd.init(ctx) || !dvc.init(ctx, wnd) || !swp.init(dvc, wnd))
        return 1;

    while (!wnd.closing())
    {
        caldera_example::Window::poll_events();
    }

    return 0;
}
