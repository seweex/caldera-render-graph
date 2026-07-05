
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <spdlog/spdlog.h>

#include <window.h>
#include <device.h>
#include <swapchain.h>
#include <shader.h>
#include <descriptor.h>
#include <memory.h>
#include <image.h>
#include <buffer.h>
#include <scheduler.h>
#include <pipeline.h>

#include <shaders/basic.frag.hpp>
#include <shaders/basic.vert.hpp>

int main()
{
    spdlog::set_level(spdlog::level::info);

    caldera_example::Context ctx;
    caldera_example::Window wnd;
    caldera_example::Device dvc;
    caldera_example::Swapchain swp;
    caldera_example::Scheduler sch;

    caldera_example::Shader vsh;
    caldera_example::Shader fsh;

    caldera_example::BindlessLayout lyt;
    caldera_example::BindlessDescriptors dsc;

    caldera_example::Allocator alc;
    caldera_example::Image img;
    caldera_example::Buffer bfr;

    caldera_example::Pipeline ppl;

    if (!ctx.init() ||
        !wnd.init(ctx) ||
        !dvc.init(ctx, wnd) ||
        !swp.init(dvc, wnd) ||
        !sch.init(dvc) ||
        !vsh.init(dvc, shader_link_compiled::spv_basic_vert) ||
        !fsh.init(dvc, shader_link_compiled::spv_basic_frag) ||
        !lyt.init(dvc) ||
        !dsc.init(dvc, lyt) ||
        !alc.init(ctx, dvc) ||
        !img.init(dvc, alc, { vk::Format::eR8G8B8A8Unorm, { 2048, 2048 }, vk::ImageUsageFlagBits::eSampled }) ||
        !bfr.init(dvc, alc, { 4096, caldera_example::Buffer::MemoryType::gpu_local, vk::BufferUsageFlagBits::eStorageBuffer }) ||
        !ppl.init(wnd, dvc, lyt, vsh, fsh))
    {
        return 1;
    }

    while (!wnd.closing())
    {
        caldera_example::Window::poll_events();

        if (!sch.begin_frame(swp))
            return 1;



        if (!sch.end_frame(swp))
            return 1;
    }

    return 0;
}
