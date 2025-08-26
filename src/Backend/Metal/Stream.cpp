#include <Backend/Metal/Stream.hpp>
#include <cassert>

namespace MMPEngine::Backend::Metal
{
    Stream::Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext)
        : Super(globalContext, streamContext)
    {
    }

    Stream::~Stream() = default;

    void Stream::Submit()
    {
        auto pool = NS::AutoreleasePool::alloc()->init();
        Super::Submit();
        pool->release();
    }

    bool Stream::ExecutionMonitorCompleted()
    {
        const auto nativeCb = _specificStreamContext->commandBuffer->GetNative();
        if(!nativeCb)
        {
            return true;
        }
        const auto status = nativeCb->status();
        return status == MTL::CommandBufferStatusCompleted;
    }

    void Stream::ResetAll()
    {
        _specificStreamContext->commandBuffer->Reset(_passControl);
    }

    void Stream::ScheduleCommandsForExecution()
    {
        const auto nativeCb = _specificStreamContext->commandBuffer->GetNative();
        assert(nativeCb);
        nativeCb->commit();
    }

    void Stream::UpdateExecutionMonitor()
    {
    }

    void Stream::WaitForExecutionMonitor()
    {
        const auto nativeCb = _specificStreamContext->commandBuffer->GetNative();
        assert(nativeCb);
        const auto status = nativeCb->status();
        if(status == MTL::CommandBufferStatusCommitted || status == MTL::CommandBufferStatusScheduled)
        {
            nativeCb->waitUntilCompleted();
        }
    }
}
