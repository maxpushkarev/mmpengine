#pragma once
#include <Backend/Shared/Context.hpp>
#include <Backend/Metal/Wrapper.hpp>

namespace MMPEngine::Backend::Metal
{
    class DeviceMemoryHeap;

    class GlobalContext : public Core::GlobalContext
    {
    public:
        GlobalContext(const Core::GlobalContext::Settings& s, const Environment& environment, std::unique_ptr<Core::Math>&& m);
        std::shared_ptr<Wrapper::Device> device;
        std::shared_ptr<Wrapper::LogState> logState;
        
        std::shared_ptr<DeviceMemoryHeap> uploadBufferHeap;
        std::shared_ptr<DeviceMemoryHeap> uniformBufferHeap;
        std::shared_ptr<DeviceMemoryHeap> readBackBufferHeap;
        std::shared_ptr<DeviceMemoryHeap> residentBufferHeap;
    };
    
    class StreamContext : public Shared::BaseStreamContext
    {
    public:
        StreamContext(
            const std::shared_ptr<Wrapper::Queue>& queue,
            const std::shared_ptr<Wrapper::CommandBuffer>& cmdBuffer
        );
        
        std::shared_ptr<Wrapper::CommandBuffer>& PopulateCommandsInBuffer();
        
        std::shared_ptr<Wrapper::Queue> queue;
        std::shared_ptr<Wrapper::CommandBuffer> commandBuffer;
    };
}
