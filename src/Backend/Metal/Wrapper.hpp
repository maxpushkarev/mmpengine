
#pragma once
#include <Metal/Metal.hpp>

namespace MMPEngine::Backend::Metal
{
    namespace Wrapper
    {
    class Device final
        {
        public:
            Device();
            Device(const Device&) = delete;
            Device(Device&&) noexcept = delete;
            Device& operator=(const Device&) = delete;
            Device& operator=(Device&&) noexcept = delete;
            ~Device();
        private:
            MTL::Device* _device = nullptr;
        };
    }
}
