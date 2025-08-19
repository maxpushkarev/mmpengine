#include <Backend/Metal/Compute.hpp>
#include <Core/Material.hpp>
#include <cassert>

namespace MMPEngine::Backend::Metal
{
    DirectComputeJob::DirectComputeJob(const std::shared_ptr<Core::ComputeMaterial>& material) : Core::DirectComputeJob(material)
    {
    }

    DirectComputeJob::~DirectComputeJob()
    {
        //TODO: destructor
    }

    DirectComputeJob::InitTask::InitTask(const std::shared_ptr<InitContext>& ctx) : Task(ctx)
    {
    }

    void DirectComputeJob::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);

        const auto ac = _specificGlobalContext;
        assert(ac);

        const auto job = GetTaskContext()->job;
        assert(job);

        job->_device = _specificGlobalContext->device;
        job->PrepareMaterialParameters(_specificGlobalContext, job->_material->GetParameters());

        //TODO: prepare init metal specific
    }

    DirectComputeJob::ExecutionTask::ExecutionTask(const std::shared_ptr<ExecutionContext>& ctx) : Task(ctx)
    {
        const auto baseJobCtx = std::make_shared<TaskContext>();
        baseJobCtx->job = std::dynamic_pointer_cast<Metal::BaseJob>(ctx->job);
        _impl = std::make_shared<Impl>(ctx);
    }

    void DirectComputeJob::ExecutionTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::OnScheduled(stream);
        stream->Schedule(_impl);
    }

    DirectComputeJob::ExecutionTask::Impl::Impl(const std::shared_ptr<ExecutionContext>& ctx) : Task(ctx)
    {
    }

    void DirectComputeJob::ExecutionTask::Impl::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);
        const auto tc = GetTaskContext();
        const auto job = tc->job;
        const auto& groups = tc->groups;
        const auto& threadsPerGroup = tc->threadsPerGroup;

        //TODO: compute metal execution impl
    }

    std::shared_ptr<Core::BaseTask> DirectComputeJob::CreateInitializationTask()
    {
        const auto ctx = std::make_shared<InitContext>();
        ctx->job = std::dynamic_pointer_cast<DirectComputeJob>(shared_from_this());
        return std::make_shared<InitTask>(ctx);
    }

    std::shared_ptr<Core::ContextualTask<Core::DirectComputeContext>> DirectComputeJob::CreateExecutionTask()
    {
        const auto ctx = std::make_shared<ExecutionContext>();
        ctx->job = std::dynamic_pointer_cast<DirectComputeJob>(shared_from_this());
        return std::make_shared<ExecutionTask>(ctx);
    }
}

