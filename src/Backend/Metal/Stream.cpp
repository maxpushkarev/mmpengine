
#include <Backend/Metal/Stream.hpp>

namespace MMPEngine::Backend::Metal
{
    Stream::Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext)
        : Core::BaseStream(globalContext, streamContext), _specificStreamContext(streamContext)
    {
    }

    void Stream::SyncInternal()
    {
        _specificStreamContext->commandBuffer->GetNative()->waitUntilCompleted();
    }

    void Stream::SubmitInternal()
    {
        _specificStreamContext->commandBuffer->GetNative()->commit();
    }

    void Stream::Flush()
    {
        _specificStreamContext->commandBuffer->GetNative()->commit();
    }
}
