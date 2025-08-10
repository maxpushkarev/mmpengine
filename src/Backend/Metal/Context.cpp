#include <Backend/Metal/Context.hpp>

namespace MMPEngine::Backend::Metal
{
    GlobalContext::GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m) : Core::GlobalContext(s, std::move(m))
    {
    }

    StreamContext::StreamContext(
        const std::shared_ptr<Wrapper::Queue>& queue,
        const std::shared_ptr<Wrapper::CommandBuffer>& cmdBuffer)
        : queue(queue), commandBuffer(cmdBuffer)
    {
    }

    std::shared_ptr<Wrapper::CommandBuffer>& StreamContext::PopulateCommandsInBuffer()
    {
        _commandsPopulated = true;
        return commandBuffer;
    }
}
