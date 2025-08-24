#include <cassert>
#include <Backend/Metal/DrawCall.hpp>

namespace MMPEngine::Backend::Metal
{
    Camera::DrawCallsJob::DrawCallsJob(const std::shared_ptr<Core::Camera>& camera, std::vector<Item>&& items)
        : Core::Camera::DrawCallsJob(camera, std::move(items))
    {
    }

    Camera::DrawCallsJob::~DrawCallsJob()
    {
        for(const auto& pair : _encodersMap)
        {
            pair.second->release();
        }
    }

    std::shared_ptr<Camera::DrawCallsJob::InternalTaskContext> Camera::DrawCallsJob::BuildInternalContext()
    {
        const auto ctx = std::make_shared<InternalTaskContext>();
        ctx->job = std::dynamic_pointer_cast<DrawCallsJob>(shared_from_this());

        ctx->colorRenderTargets.clear();
        ctx->colorRenderTargets.reserve(ctx->job->_camera->GetTarget().color.size());

        const auto ds = ctx->job->_camera->GetTarget().depthStencil;

        for (const auto& crt : ctx->job->_camera->GetTarget().color)
        {
            ctx->colorRenderTargets.push_back(std::dynamic_pointer_cast<IColorTargetTexture>(crt.tex->GetUnderlyingTexture()));
        }

        if (ds.tex)
        {
            ctx->depthStencil = std::dynamic_pointer_cast<IDepthStencilTexture>(ds.tex->GetUnderlyingTexture());
        }

        return ctx;
    }


    std::shared_ptr<Camera::DrawCallsJob::Iteration> Camera::DrawCallsJob::BuildIteration(const Item& item) const
    {
        if (std::dynamic_pointer_cast<Core::MeshMaterial>(item.material))
        {
            return std::make_shared<IterationJob<Core::MeshMaterial>>(std::const_pointer_cast<DrawCallsJob>(std::dynamic_pointer_cast<const DrawCallsJob>(shared_from_this())), item);
        }

        return nullptr;
    }

    std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::CreateInitializationTaskInternal()
    {
        return std::make_shared<InitInternalTask>(BuildInternalContext());
    }

    std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::CreateTaskForIterationsStart()
    {
        return std::make_shared<BeginPass>(BuildInternalContext());
    }

    std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::CreateTaskForIterationsFinish()
    {
        return std::make_shared<EndPass>(BuildInternalContext());
    }

    Camera::DrawCallsJob::InitInternalTask::InitInternalTask(const std::shared_ptr<InternalTaskContext>& ctx) : Task<MMPEngine::Backend::Metal::Camera::DrawCallsJob::InternalTaskContext>(ctx)
    {
    }

    void Camera::DrawCallsJob::InitInternalTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);
        const auto ctx = GetTaskContext();
    }
    Camera::DrawCallsJob::BeginPass::BeginPass(const std::shared_ptr<InternalTaskContext>& ctx) : Task(ctx)
    {
    }

    void Camera::DrawCallsJob::BeginPass::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);

        const auto tc = GetTaskContext();
        
        /*const auto renderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();
        auto l = tc->colorRenderTargets[0]->GetNativeTexture();
        
        auto colorAttachment = renderPassDescriptor->colorAttachments()->object(0);
            colorAttachment->setTexture(tc->colorRenderTargets[0]->GetNativeTexture());
            colorAttachment->setLoadAction(MTL::LoadActionClear);
            colorAttachment->setStoreAction(MTL::StoreActionStore);
            colorAttachment->setClearColor(MTL::ClearColor(0.1, 0.2, 0.3, 1.0)); // R,G,B,A
        
        const auto currentSyncCounter = stream->GetSyncCounterValue();
        const auto lastCompletedSyncCounter = stream->GetLastCompletedSyncCounterValue();
        
        tc->job->_encodersReleaseIds.clear();
        
        for(const auto& pair : tc->job->_encodersMap)
        {
            if(pair.first <= lastCompletedSyncCounter)
            {
                tc->job->_encodersReleaseIds.push_back(pair.first);
            }
        }
        
        for(const auto id : tc->job->_encodersReleaseIds)
        {
            assert(tc->job->_encodersMap[id]->retainCount() == 1);
            tc->job->_encodersMap[id]->release();
            tc->job->_encodersMap.erase(id);
        }
        
        tc->job->_currentRenderCommandEncoder = _specificStreamContext->PopulateCommandsInBuffer()->GetNative()->renderCommandEncoder(renderPassDescriptor);
        tc->job->_encodersMap[currentSyncCounter] = tc->job->_currentRenderCommandEncoder;
        
        renderPassDescriptor->release();*/
    }

    Camera::DrawCallsJob::EndPass::EndPass(const std::shared_ptr<InternalTaskContext>& ctx) : Task<MMPEngine::Backend::Metal::Camera::DrawCallsJob::InternalTaskContext>(ctx)
    {
    }

    void Camera::DrawCallsJob::EndPass::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);
        const auto tc = GetTaskContext();
        
        //tc->job->_currentRenderCommandEncoder->endEncoding();
    }

    Camera::DrawCallsJob::IterationImpl::IterationImpl(const std::shared_ptr<DrawCallsJob>& job, const Item& item) : _item(item)
    {
    }
}
