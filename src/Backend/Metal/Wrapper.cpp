#include <Backend/Metal/Wrapper.hpp>
#include <Backend/Metal/Context.hpp>
#include <iostream>
#include <cassert>

namespace MMPEngine::Backend::Metal
{
    namespace Wrapper
    {
        Device::Device()
        {
            _device = NS::TransferPtr(MTL::CreateSystemDefaultDevice());
        }
    
        MTL::Device* Device::GetNative() const
        {
            return _device.get();
        }
    
        LogState::LogState(const std::shared_ptr<Device>& device, std::uint32_t bufferSize, MTL::LogLevel logLevel) : _device(device)
        {
            auto logStateDesc = MTL::LogStateDescriptor::alloc()->init();
            logStateDesc->setBufferSize(static_cast<NS::Integer>(bufferSize));
            logStateDesc->setLevel(logLevel);
            
            NS::Error* err = nullptr;
            _logState = NS::TransferPtr(_device->GetNative()->newLogState(logStateDesc, &err));
            assert(err == nullptr);
            
            logStateDesc->release();
        }
        
        MTL::LogState* LogState::GetNative() const
        {
            return _logState.get();
        }
    
        Queue::Queue(const std::shared_ptr<GlobalContext>& globalContext, std::uint32_t maxCmdBuffersCount) : _globalContext(globalContext)
        {
            auto commandQueueDesc = MTL::CommandQueueDescriptor::alloc()->init();
            commandQueueDesc->setLogState(_globalContext->logState->GetNative());
            commandQueueDesc->setMaxCommandBufferCount(static_cast<NS::UInteger>(maxCmdBuffersCount));
            
            _queue = NS::TransferPtr(_globalContext->device->GetNative()->newCommandQueue(commandQueueDesc));
            
            commandQueueDesc->release();
        }
    
        MTL::CommandQueue* Queue::GetNative() const
        {
            return  _queue.get();
        }
    
        CommandBuffer::CommandBuffer(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<Queue>& queue) : _queue(queue), _globalContext(globalContext)
        {
            _commandBufferDescriptor = MTL::CommandBufferDescriptor::alloc()->init();
            _commandBufferDescriptor->setRetainedReferences(false);
            _commandBufferDescriptor->setLogState(_globalContext->logState->GetNative());
            _commandBufferDescriptor->setErrorOptions(_globalContext->settings.isDebug ? MTL::CommandBufferErrorOptionEncoderExecutionStatus : MTL::CommandBufferErrorOptionNone);
        }

        CommandBuffer::~CommandBuffer()
        {
            if(_commandBuffer)
            {
                _commandBuffer->release();
                _commandBuffer = nullptr;
            }
            
            if(_commandBufferDescriptor)
            {
                _commandBufferDescriptor->release();
            }
        }

        MTL::CommandBuffer* CommandBuffer::GetNative() const
        {
            return  _commandBuffer;
        }
    
        void CommandBuffer::Reset(PassControl)
        {
            if(_commandBuffer)
            {
                _commandBuffer->release();
                _commandBuffer = nullptr;
            }
            
            _commandBuffer = _queue->GetNative()->commandBuffer(_commandBufferDescriptor);
            _commandBuffer->retain();
            
            assert(_commandBuffer != nullptr);
        }
    
    }
}
