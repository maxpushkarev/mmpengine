#include <Backend/Metal/Wrapper.hpp>
#include <iostream>
#include <cassert>

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
    
        LogState::LogState(const std::shared_ptr<Device>& device, MTL::LogStateDescriptor* desc) : _device(device)
        {
            NS::Error* err = nullptr;
            _logState = _device->GetNative()->newLogState(desc, &err);
            assert(err == nullptr);
        }
        
        MTL::LogState* LogState::GetNative() const
        {
            return _logState;
        }
    
        LogState::~LogState()
        {
            if(_logState)
            {
                _logState->release();
            }
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
    
        CommandBuffer::CommandBuffer(const std::shared_ptr<Queue>& queue, MTL::CommandBufferDescriptor* cbDesc) : _queue(queue)
        {
            _commandBuffer = _queue->GetNative()->commandBuffer(cbDesc);
        }

        CommandBuffer::~CommandBuffer()
        {
            if(_commandBuffer)
            {
                _commandBuffer->release();
            }
        }

        MTL::CommandBuffer* CommandBuffer::GetNative() const
        {
            return  _commandBuffer;
        }
    }
}
