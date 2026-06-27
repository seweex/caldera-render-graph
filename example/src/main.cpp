
#include <iostream>

#include <window.h>
#include <device.h>

int main()
{
    caldera_example::Context ctx;
    caldera_example::Window wnd;
    caldera_example::Device dvc;

    if (!ctx.init() || !wnd.init(ctx) || !dvc.init(ctx, wnd))
        return 1;

    while (!wnd.closing())
    {
        caldera_example::Window::poll_events();
    }

    return 0;
}
