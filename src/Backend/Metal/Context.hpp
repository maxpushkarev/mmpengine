#pragma once
#include <Core/Context.hpp>
#include <Backend/Shared/Context.hpp>
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
    
    class StreamContext : public Shared::StreamContext<
        std::shared_ptr<Wrapper::Queue>,
        std::shared_ptr<Wrapper::DummyCommandBufferAllocator>,
        std::shared_ptr<Wrapper::CommandBuffer>,
        std::shared_ptr<Wrapper::DummyFence>>
    {
    public:
        StreamContext(
            const std::shared_ptr<Wrapper::Queue>& queue,
            const std::shared_ptr<Wrapper::DummyCommandBufferAllocator>& allocator,
            const std::shared_ptr<Wrapper::CommandBuffer>& cmdBuffer,
            const std::shared_ptr<Wrapper::DummyFence>& fence
        );
    };
}
