#include <Backend/Metal/Texture.hpp>
#include <cassert>

namespace MMPEngine::Backend::Metal
{
    ITargetTexture::ITargetTexture() = default;
    ITargetTexture::~ITargetTexture() = default;

    ResourceTexture::ResourceTexture() = default;
    ResourceTexture::~ResourceTexture()
    {
        if(_nativeTexture)
        {
            _nativeTexture->release();
        }
    }

    std::shared_ptr<DeviceMemoryHeap> ResourceTexture::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
    {
        return globalContext->residentBufferHeap;
    }

    DepthStencilTargetTexture::DepthStencilTargetTexture(const Settings& settings) : Core::DepthStencilTargetTexture(settings)
    {
    }

    std::shared_ptr<Core::BaseTask> DepthStencilTargetTexture::CreateInitializationTask()
    {
        const auto ctx = std::make_shared<InitTaskContext>();
        ctx->entity = std::dynamic_pointer_cast<DepthStencilTargetTexture>(shared_from_this());
        return std::make_shared<InitTask>(ctx);
    }

    DepthStencilTargetTexture::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task(ctx)
    {
    }

    void DepthStencilTargetTexture::InitTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::OnScheduled(stream);

        stream->Schedule(std::make_shared<Allocate>(GetTaskContext()));
        stream->Schedule(GetTaskContext()->entity->GetMemoryHeap(_specificGlobalContext)->CreateTaskToInitializeBlocks());
        stream->Schedule(std::make_shared<Create>(GetTaskContext()));
    }

    DepthStencilTargetTexture::InitTask::Allocate::Allocate(const std::shared_ptr<InitTaskContext>& context)
        : Task<MMPEngine::Backend::Metal::DepthStencilTargetTexture::InitTaskContext>(context)
    {
    }

    void DepthStencilTargetTexture::InitTask::Allocate::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::OnScheduled(stream);

        const auto dsTex = GetTaskContext()->entity;
        
    }

    DepthStencilTargetTexture::InitTask::Create::Create(const std::shared_ptr<InitTaskContext>& context)
        : Task<MMPEngine::Backend::Metal::DepthStencilTargetTexture::InitTaskContext>(context)
    {
    }

    void DepthStencilTargetTexture::InitTask::Create::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);

        const auto dsTex = GetTaskContext()->entity;
    }

}
