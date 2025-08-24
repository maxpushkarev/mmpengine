#pragma once
#include <QuartzCore/QuartzCore.hpp>

namespace MMPEngine::Backend::Metal
{
    class Window
    {
    public:
        static void JoinMetalLayerToWindow(void* nativeWindow, CA::MetalLayer* metalLayer);
    };
}
