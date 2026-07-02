
#include <iostream>

#include <window.h>
#include <device.h>
#include <swapchain.h>
#include <frame.h>
#include <shader.h>

#include <shaders/basic.frag.hpp>
#include <shaders/basic.vert.hpp>

int main()
{
    caldera_example::Context ctx;
    caldera_example::Window wnd;
    caldera_example::Device dvc;
    caldera_example::Swapchain swp;
    caldera_example::FrameManager fmg;

    caldera_example::Shader vsh;
    caldera_example::Shader fsh;

    if (!ctx.init() ||
        !wnd.init(ctx) ||
        !dvc.init(ctx, wnd) ||
        !swp.init(dvc, wnd) ||
        !fmg.init(dvc) ||
        !vsh.init(dvc, shader_link_compiled::spv_basic_vert) ||
        !fsh.init(dvc, shader_link_compiled::spv_basic_frag))
    {
        return 1;
    }

    while (!wnd.closing())
    {
        caldera_example::Window::poll_events();
    }

    return 0;
}
