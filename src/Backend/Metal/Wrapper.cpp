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
    
        MTL::Device* Device::GetNative() const
        {
            return _device;
        }
    
        Queue::Queue(const std::shared_ptr<Device>& device, MTL::CommandQueueDescriptor* queueDesc) : _device(device)
        {
            _queue = _device->GetNative()->newCommandQueue(queueDesc);
        }
    
        Queue::~Queue()
        {
            if(_queue)
            {
                _queue->release();
            }
        }
    
        MTL::CommandQueue* Queue::GetNative() const
        {
            return  _queue;
        }
    }
}
