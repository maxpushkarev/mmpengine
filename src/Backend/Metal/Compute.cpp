#include <Backend/Metal/Compute.hpp>
#include <Backend/Metal/Shader.hpp>
#include <Core/Material.hpp>
#include <cassert>

namespace MMPEngine::Backend::Metal
{
    DirectComputeJob::DirectComputeJob(const std::shared_ptr<Core::ComputeMaterial>& material) : Core::DirectComputeJob(material)
    {
    }

    DirectComputeJob::~DirectComputeJob()
    {
        if(_computePipelineState)
        {
            _computePipelineState->release();
        }
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

        job->PrepareMaterialParameters(_specificGlobalContext, job->_material->GetParameters());

        const auto shader = std::dynamic_pointer_cast<Metal::LibShader>(job->_material->GetShader());
        
        NS::Error* err = nullptr;
        job->_computePipelineState = _specificGlobalContext->device->GetNative()->newComputePipelineState(shader->GetNativeFunction(), &err);
    
        
        assert(err == nullptr);
        assert(job->_computePipelineState);
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

        const auto computeEncoder = _specificStreamContext->PopulateCommandsInBuffer()->GetNative()->computeCommandEncoder(MTL::DispatchTypeSerial);
        
        computeEncoder->setComputePipelineState(tc->job->_computePipelineState);
        
        NS::UInteger idx = 0;
        for(const auto& bufferData : tc->job->_bufferDataCollection)
        {
            if(bufferData.second.has_value()){
                computeEncoder->setBuffer(bufferData.first->GetNative(), 0, bufferData.second.value(), idx);
            }
            else{
                computeEncoder->setBuffer(bufferData.first->GetNative(), 0, idx);
            }
            ++idx;
        }
        
        computeEncoder->dispatchThreadgroups(
            MTL::Size{
                static_cast<NS::UInteger>(groups.x),
                static_cast<NS::UInteger>(groups.y),
                static_cast<NS::UInteger>(groups.z)
            },
            MTL::Size{
                static_cast<NS::UInteger>(threadsPerGroup.x),
                static_cast<NS::UInteger>(threadsPerGroup.y),
                static_cast<NS::UInteger>(threadsPerGroup.z)
            }
        );

        computeEncoder->endEncoding();
        computeEncoder->release();
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

