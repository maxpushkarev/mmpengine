#pragma once
#include <Core/Context.hpp>
#include <Backend/Metal/Wrapper.hpp>

namespace MMPEngine::Backend::Metal
{
    class GlobalContext : public Core::GlobalContext
    {
    public:
        GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m);
        std::shared_ptr<Wrapper::Device> device;
        std::shared_ptr<Wrapper::LogState> logState;
    };
    
    class StreamContext : public Core::StreamContext
    {
    public:
        StreamContext(
            const std::shared_ptr<Wrapper::Queue>& queue,
            const std::shared_ptr<Wrapper::CommandBuffer>& cmdBuffer
        );
        
        std::shared_ptr<Wrapper::Queue> queue;
        std::shared_ptr<Wrapper::CommandBuffer> commandBuffer;
    };
}
