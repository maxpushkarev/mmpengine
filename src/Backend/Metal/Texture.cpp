#include <Backend/Metal/Texture.hpp>
#include <cassert>

namespace MMPEngine::Backend::Metal
{
    ITargetTexture::ITargetTexture() = default;
    ITargetTexture::~ITargetTexture() = default;


    NS::UInteger ITargetTexture::GetSamplesCount(Core::TargetTexture::Settings::Antialiasing aa)
    {
        auto res = static_cast<NS::UInteger>(aa);
        if(aa == Core::TargetTexture::Settings::Antialiasing::MSAA_0)
        {
            res = 1U;
        }
        return res;
    }

    ResourceTexture::ResourceTexture() = default;
    ResourceTexture::~ResourceTexture()
    {
        if(_nativeTextureDesc)
        {
            _nativeTextureDesc->release();
        }
        
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

    MTL::PixelFormat DepthStencilTargetTexture::GetFormat() const
    {
        const auto& settings = GetSettings();

        switch (settings.format)
        {
        case Settings::Format::Depth16:
            return MTL::PixelFormatDepth16Unorm;
        case Settings::Format::Depth32:
            return MTL::PixelFormatDepth32Float;
        case Settings::Format::Depth24_Stencil8:
                if(_device->GetNative()->depth24Stencil8PixelFormatSupported())
                {
                    return MTL::PixelFormatDepth24Unorm_Stencil8;
                }
                else
                {
                    return MTL::PixelFormatDepth32Float_Stencil8;
                }
        }

        throw Core::UnsupportedException("unknown depth/stencil format");
    }


    NS::UInteger DepthStencilTargetTexture::GetSamplesCount() const
    {
        return ITargetTexture::GetSamplesCount(_settings.base.antialiasing);
    }

    MTL::Texture* DepthStencilTargetTexture::GetNativeTexture() const
    {
        return _nativeTexture;
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
        const auto tc = GetTaskContext();
        const auto entity = tc->entity;
        const auto memHeap = entity->GetMemoryHeap(_specificGlobalContext);
        
        entity->_device = _specificGlobalContext->device;
        entity->_nativeTextureDesc = MTL::TextureDescriptor::alloc()->init();
        entity->_nativeTextureDesc->setUsage(MTL::TextureUsageRenderTarget);
        entity->_nativeTextureDesc->setStorageMode(memHeap->GetMtlSettings().storageMode);
        entity->_nativeTextureDesc->setResourceOptions(memHeap->GetMtlSettings().resourceOption);
        entity->_nativeTextureDesc->setCpuCacheMode(memHeap->GetMtlSettings().cpuCacheMode);
        entity->_nativeTextureDesc->setTextureType(MTL::TextureType2D);
        entity->_nativeTextureDesc->setWidth(static_cast<NS::UInteger>(tc->entity->_settings.base.size.x));
        entity->_nativeTextureDesc->setHeight(static_cast<NS::UInteger>(tc->entity->_settings.base.size.y));
        entity->_nativeTextureDesc->setHazardTrackingMode(MTL::HazardTrackingModeTracked);
        entity->_nativeTextureDesc->setAllowGPUOptimizedContents(true);
        entity->_nativeTextureDesc->setMipmapLevelCount(1);
        entity->_nativeTextureDesc->setDepth(1);
        entity->_nativeTextureDesc->setArrayLength(1);
        entity->_nativeTextureDesc->setCompressionType(MTL::TextureCompressionTypeLossless);
        entity->_nativeTextureDesc->setSwizzle(MTL::TextureSwizzleChannels(MTL::TextureSwizzleRed,
                                                        MTL::TextureSwizzleGreen,
                                                        MTL::TextureSwizzleBlue,
                                                        MTL::TextureSwizzleAlpha ));
        
        entity->_nativeTextureDesc->setPixelFormat(entity->GetFormat());
        entity->_nativeTextureDesc->setSampleCount(entity->GetSamplesCount());
        
       const auto sizeAndAlign = _specificGlobalContext->device->GetNative()->heapTextureSizeAndAlign(entity->_nativeTextureDesc);
        
        tc->entity->_deviceMemoryHeapHandle = memHeap->Allocate(Core::Heap::Request {
            static_cast<std::size_t>(sizeAndAlign.size),
                static_cast<std::size_t>(sizeAndAlign.align)
        });
        
    }

    DepthStencilTargetTexture::InitTask::Create::Create(const std::shared_ptr<InitTaskContext>& context)
        : Task<MMPEngine::Backend::Metal::DepthStencilTargetTexture::InitTaskContext>(context)
    {
    }

    void DepthStencilTargetTexture::InitTask::Create::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::OnComplete(stream);
        
        const auto tc = GetTaskContext();
        if(tc->entity->_nativeTextureDesc)
        {
            tc->entity->_nativeTextureDesc->release();
            tc->entity->_nativeTextureDesc = nullptr;
        }
    }

    void DepthStencilTargetTexture::InitTask::Create::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);

        const auto tc = GetTaskContext();
        tc->entity->_nativeTexture = tc->entity->_deviceMemoryHeapHandle.GetMemoryBlock()->GetNative()->newTexture(tc->entity->_nativeTextureDesc, static_cast<NS::UInteger>(tc->entity->_deviceMemoryHeapHandle.GetOffset()));
    }
}
