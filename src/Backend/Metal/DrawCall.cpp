#include <cassert>
#include <Backend/Metal/DrawCall.hpp>

namespace MMPEngine::Backend::Metal
{
    Camera::DrawCallsJob::DrawCallsJob(const std::shared_ptr<Core::Camera>& camera, std::vector<Item>&& items)
        : Core::Camera::DrawCallsJob(camera, std::move(items))
    {
    }

    Camera::DrawCallsJob::~DrawCallsJob() = default;

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
        
        const auto renderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();
        renderPassDescriptor->setRenderTargetArrayLength(static_cast<NS::UInteger>(tc->colorRenderTargets.size()));
        
        auto first = tc->colorRenderTargets[0];
        renderPassDescriptor->setRenderTargetWidth(static_cast<NS::UInteger>(first->GetNativeTexture()->width()));
        renderPassDescriptor->setRenderTargetHeight(static_cast<NS::UInteger>(first->GetNativeTexture()->height()));
        
        for (std::size_t i = 0; i < tc->colorRenderTargets.size(); ++i)
        {
            const auto& crt = tc->colorRenderTargets[i];
            
            auto mtlColorAttachment = renderPassDescriptor->colorAttachments()->object(0);
            mtlColorAttachment->setTexture(crt->GetNativeTexture());
            
            mtlColorAttachment->setStoreAction(MTL::StoreActionStore);
            mtlColorAttachment->setLoadAction(MTL::LoadActionLoad);
            
            if (tc->job->_camera->GetTarget().color[i].clear && tc->job->_camera->GetTarget().color[i].tex->GetSettings().clearColor.has_value())
            {
                mtlColorAttachment->setLoadAction(MTL::LoadActionClear);
                
                const auto& clearColor = tc->job->_camera->GetTarget().color[i].tex->GetSettings().clearColor.value();
                mtlColorAttachment->setClearColor(MTL::ClearColor {clearColor.x, clearColor.y, clearColor.z, clearColor.w});
            }
            
            if(tc->depthStencil)
            {
                auto rpDepthAttachmentDesc = MTL::RenderPassDepthAttachmentDescriptor::alloc()->init();
                
                rpDepthAttachmentDesc->setTexture(tc->depthStencil->GetNativeTexture());
                
                rpDepthAttachmentDesc->setStoreAction(MTL::StoreActionStore);
                rpDepthAttachmentDesc->setLoadAction(MTL::LoadActionLoad);
                
                if(tc->job->_camera->GetTarget().depthStencil.clear &&
                   tc->job->_camera->GetTarget().depthStencil.tex->GetSettings().clearValue.has_value())
                {
                    const auto& clearValue = tc->job->_camera->GetTarget().depthStencil.tex->GetSettings().clearValue.value();
                    
                    rpDepthAttachmentDesc->setLoadAction(MTL::LoadActionClear);
                    rpDepthAttachmentDesc->setClearDepth(static_cast<double>(std::get<0>(clearValue)));
                }
                
                renderPassDescriptor->setDepthAttachment(rpDepthAttachmentDesc);
                rpDepthAttachmentDesc->release();
                
                
                if(tc->job->_camera->GetTarget().depthStencil.tex->StencilIncluded())
                {
                    auto rpStencilAttachmentDesc = MTL::RenderPassStencilAttachmentDescriptor::alloc()->init();
                    
                    rpStencilAttachmentDesc->setTexture(tc->depthStencil->GetNativeTexture());
                    
                    rpStencilAttachmentDesc->setStoreAction(MTL::StoreActionStore);
                    rpStencilAttachmentDesc->setLoadAction(MTL::LoadActionLoad);
                    
                    if(tc->job->_camera->GetTarget().depthStencil.clear &&
                       tc->job->_camera->GetTarget().depthStencil.tex->GetSettings().clearValue.has_value())
                    {
                        const auto& clearValue = tc->job->_camera->GetTarget().depthStencil.tex->GetSettings().clearValue.value();
                        
                        rpStencilAttachmentDesc->setLoadAction(MTL::LoadActionClear);
                        rpStencilAttachmentDesc->setClearStencil(static_cast<std::uint32_t>(std::get<1>(clearValue)));
                    }
                    
                    renderPassDescriptor->setStencilAttachment(rpStencilAttachmentDesc);
                    rpStencilAttachmentDesc->release();
                }
            }
        }
        
        tc->job->_renderCommandEncoder = _specificStreamContext->PopulateCommandsInBuffer()->GetNative()->renderCommandEncoder(renderPassDescriptor);
        
        renderPassDescriptor->release();
    }

    Camera::DrawCallsJob::EndPass::EndPass(const std::shared_ptr<InternalTaskContext>& ctx) : Task<MMPEngine::Backend::Metal::Camera::DrawCallsJob::InternalTaskContext>(ctx)
    {
    }

    void Camera::DrawCallsJob::EndPass::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);
        const auto tc = GetTaskContext();
        
        tc->job->_renderCommandEncoder->endEncoding();
    }

    Camera::DrawCallsJob::IterationImpl::IterationImpl(const std::shared_ptr<DrawCallsJob>& job, const Item& item) : _item(item)
    {
    }
}
