#ifndef CALDERA_EXAMPLES_COMMON_PIPELINE_H
#define CALDERA_EXAMPLES_COMMON_PIPELINE_H

#include <caldera-examples-common/config.h>

namespace caldera_examples_common
{
    struct Window;
    struct Device;
    struct Shader;
    struct LayoutProxy;

    struct Pipeline
    {
        Pipeline() noexcept;
        ~Pipeline() noexcept;

        Pipeline(Pipeline &&) = delete;
        Pipeline& operator=(Pipeline &&) = delete;

        Pipeline(Pipeline const&) = delete;
        Pipeline& operator=(Pipeline const&) = delete;

        [[nodiscard]] bool init(
            Window const& window,
            Device const& device,
            LayoutProxy layouts,
            Shader const& vertex,
            Shader const& fragment);

        void clear() noexcept;

    private:
        vk::Device m_device;

    public:
        vk::Pipeline pipeline;
    };
}

#endif
