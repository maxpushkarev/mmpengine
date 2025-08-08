#include <Backend/Metal/Wrapper.hpp>
#include <iostream>

namespace MMPEngine::Backend::Metal
{
    namespace Wrapper
    {
        Device::Device()
        {
            _device = MTL::CreateSystemDefaultDevice();
        }

        Device::~Device()
        {
            if(_device)
            {
                _device->release();
            }
        }
    }
}
