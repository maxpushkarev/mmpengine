#pragma once
#include <Core/Stream.hpp>
#include <Backend/Metal/Context.hpp>
#include <Backend/Shared/Stream.hpp>

namespace MMPEngine::Backend::Metal
{
    class Stream : public Shared::Stream<
        GlobalContext,
        StreamContext>
    {
    private:
        using Super = Shared::Stream<
            GlobalContext,
            StreamContext>;
    public:
        Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext);
        Stream(const Stream&) = delete;
        Stream(Stream&&) noexcept = delete;
        Stream& operator=(const Stream&) = delete;
        Stream& operator=(Stream&&) noexcept = delete;
        ~Stream() override;
    protected:
        bool ExecutionMonitorCompleted() override;
        void ResetAll() override;
        void ScheduleCommandsForExecution() override;
        void UpdateExecutionMonitor() override;
        void WaitForExecutionMonitor() override;
    };
}
