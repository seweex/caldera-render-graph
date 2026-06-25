
#include <iostream>
#include <window.h>

int main()
{
    caldera_example::Context ctx;
    caldera_example::Window wnd;

    if (!ctx.init() || !wnd.init())
        return 1;

    while (!wnd.closing())
    {
        caldera_example::Window::poll_events();
    }

    return 0;
}