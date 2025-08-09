#include <Backend/Metal/Context.hpp>

namespace MMPEngine::Backend::Metal
{
    GlobalContext::GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m) : Core::GlobalContext(s, std::move(m))
    {
    }

    StreamContext::StreamContext(
        const std::shared_ptr<Wrapper::Queue>& queue,
        const std::shared_ptr<Wrapper::DummyCommandBufferAllocator>& allocator,
        const std::shared_ptr<Wrapper::CommandBuffer>& cmdBuffer,
        const std::shared_ptr<Wrapper::DummyFence>& fence)
            : Shared::StreamContext<
                std::shared_ptr<MMPEngine::Backend::Metal::Wrapper::Queue>,
                std::shared_ptr<MMPEngine::Backend::Metal::Wrapper::DummyCommandBufferAllocator>,
                std::shared_ptr<MMPEngine::Backend::Metal::Wrapper::CommandBuffer>,
                std::shared_ptr<MMPEngine::Backend::Metal::Wrapper::DummyFence>>(queue, allocator, cmdBuffer, fence)
    {
    }
}
