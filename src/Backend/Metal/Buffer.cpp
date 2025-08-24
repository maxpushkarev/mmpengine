#include <Backend/Metal/Buffer.hpp>
#include <cassert>

namespace MMPEngine::Backend::Metal
{
    Buffer::Buffer(const MTLSettings& mtlSettings) : _mtlSettings(mtlSettings)
    {
    }

    MTL::Buffer* Buffer::GetNative() const
    {
        return _nativeBuffer.get();
    }

    Buffer::InitTask::Allocate::Allocate(const std::shared_ptr<InitTaskContext>& context) : Task<MMPEngine::Backend::Metal::Buffer::InitTaskContext>(context)
    {
    }

    void Buffer::InitTask::Allocate::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::OnScheduled(stream);

        const auto tc = GetTaskContext();
        const auto memHeap = tc->entity->GetMemoryHeap(_specificGlobalContext);
        
        const auto sizeAndAlign = _specificGlobalContext->device->GetNative()->heapBufferSizeAndAlign(static_cast<NS::UInteger>(tc->byteSize), memHeap->GetMtlSettings().resourceOption);
        
        tc->entity->_deviceMemoryHeapHandle = memHeap->Allocate(Core::Heap::Request {
            static_cast<std::size_t>(sizeAndAlign.size),
                static_cast<std::size_t>(sizeAndAlign.align)
        });
    }

    Buffer::InitTask::Create::Create(const std::shared_ptr<InitTaskContext>& context) : Task<MMPEngine::Backend::Metal::Buffer::InitTaskContext>(context)
    {
    }

    void Buffer::InitTask::Create::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);

        const auto tc = GetTaskContext();
        const auto memHeap = tc->entity->GetMemoryHeap(_specificGlobalContext);
        
        tc->entity->_nativeBuffer = NS::TransferPtr(tc->entity->_deviceMemoryHeapHandle.GetMemoryBlock()->GetNative()->newBuffer(static_cast<NS::UInteger>(tc->entity->_deviceMemoryHeapHandle.GetSize()), memHeap->GetMtlSettings().resourceOption, static_cast<NS::UInteger>(tc->entity->_deviceMemoryHeapHandle.GetOffset())));
    }


    Buffer::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& context) : Task<MMPEngine::Backend::Metal::Buffer::InitTaskContext>(context)
    {
    }

    void Buffer::InitTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::OnScheduled(stream);
        stream->Schedule(std::make_shared<Allocate>(GetTaskContext()));
        stream->Schedule(GetTaskContext()->entity->GetMemoryHeap(_specificGlobalContext)->CreateTaskToInitializeBlocks());
        stream->Schedule(std::make_shared<Create>(GetTaskContext()));
    }

    Buffer::CopyBufferTask::CopyBufferTask(const std::shared_ptr<CopyBufferTaskContext>& context) : Task<
        MMPEngine::Backend::Metal::Buffer::CopyBufferTaskContext>(context)
    {
    }

    void Buffer::CopyBufferTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);

        const auto tc = GetTaskContext();
        const auto srcBuffer = tc->src;
        const auto dstBuffer = tc->dst;
        
        const auto blitEncoder = _specificStreamContext->PopulateCommandsInBuffer()->GetNative()->blitCommandEncoder();
        
        blitEncoder->copyFromBuffer(
            srcBuffer->_nativeBuffer.get(),
            static_cast<NS::UInteger>(tc->srcByteOffset),
            dstBuffer->_nativeBuffer.get(),
            static_cast<NS::UInteger>(tc->dstByteOffset),
            static_cast<NS::UInteger>(tc->byteLength)
        );
        
        blitEncoder->endEncoding();
    }

    UploadBuffer::UploadBuffer(const Settings& settings) : Core::UploadBuffer(settings), Metal::Buffer(MTLSettings {})
    {
    }

    UploadBuffer::WriteTask::WriteTask(const std::shared_ptr<WriteTaskContext>& context) : Task(context)
    {
        const auto buffer = context->uploadBuffer;
        assert(buffer);

        _implTask = std::make_shared<Impl>(context);
    }

    void UploadBuffer::WriteTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::OnScheduled(stream);

        stream->Schedule(Core::StreamBarrierTask::kInstance);
        stream->Schedule(_implTask);
    }

    UploadBuffer::WriteTask::Impl::Impl(const std::shared_ptr<WriteTaskContext>& context) : Task(context)
    {
    }

    void UploadBuffer::WriteTask::Impl::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);
        if (const auto tc = GetTaskContext(); const auto entity = tc->uploadBuffer)
        {
            std::memcpy(static_cast<char*>(entity->_nativeBuffer->contents()) + tc->byteOffset, tc->src, tc->byteLength);
        }
    }


    std::shared_ptr<Core::ContextualTask<Core::UploadBuffer::WriteTaskContext>> UploadBuffer::CreateWriteTask(const void* src, std::size_t byteLength, std::size_t byteOffset)
    {
        const auto ctx = std::make_shared<WriteTaskContext>();
        ctx->uploadBuffer = std::dynamic_pointer_cast<UploadBuffer>(shared_from_this());
        ctx->byteOffset = byteOffset;
        ctx->byteLength = byteLength;
        ctx->src = src;

        return std::make_shared<WriteTask>(ctx);
    }

    std::shared_ptr<Core::BaseTask> UploadBuffer::CreateCopyToBufferTask(
        const std::shared_ptr<Core::Buffer>& dst,
        std::size_t byteLength,
        std::size_t srcByteOffset,
        std::size_t dstByteOffset) const
    {
        const auto context = std::make_shared<CopyBufferTaskContext>();
        context->src = std::dynamic_pointer_cast<Metal::Buffer>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
        context->dst = std::dynamic_pointer_cast<Metal::Buffer>(dst->GetUnderlyingBuffer());
        context->srcByteOffset = srcByteOffset;
        context->dstByteOffset = dstByteOffset;
        context->byteLength = byteLength;

        return std::make_shared<CopyBufferTask>(context);
    }

    std::shared_ptr<Core::BaseTask> UploadBuffer::CreateInitializationTask()
    {
        const auto ctx = std::make_shared<InitTaskContext>();
        ctx->byteSize = GetSettings().byteLength;
        ctx->entity = std::dynamic_pointer_cast<Metal::Buffer>(shared_from_this());
        return std::make_shared<InitTask>(ctx);
    }

    std::shared_ptr<DeviceMemoryHeap> UploadBuffer::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
    {
        return globalContext->uploadBufferHeap;
    }

    ReadBackBuffer::ReadBackBuffer(const Settings& settings) : Core::ReadBackBuffer(settings), Metal::Buffer(MTLSettings{})
    {
    }

    ReadBackBuffer::ReadTask::ReadTask(const std::shared_ptr<ReadTaskContext>& context) : Task(context)
    {
        const auto buffer = context->readBackBuffer;
        assert(buffer);

        _implTask = std::make_shared<Impl>(context);
    }

    void ReadBackBuffer::ReadTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::OnScheduled(stream);

        stream->Schedule(Core::StreamBarrierTask::kInstance);
        stream->Schedule(_implTask);
    }

    ReadBackBuffer::ReadTask::Impl::Impl(const std::shared_ptr<ReadTaskContext>& context) : Task(context)
    {
    }

    void ReadBackBuffer::ReadTask::Impl::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);

        if (const auto tc = GetTaskContext(); const auto entity = tc->readBackBuffer)
        {
            std::memcpy(tc->dst, static_cast<char*>(entity->_nativeBuffer->contents()) + tc->byteOffset, tc->byteLength);
        }
    }

    std::shared_ptr<Core::ContextualTask<Core::ReadBackBuffer::ReadTaskContext>> ReadBackBuffer::CreateReadTask(void* dst, std::size_t byteLength, std::size_t byteOffset)
    {
        const auto ctx = std::make_shared<ReadTaskContext>();
        ctx->readBackBuffer = std::dynamic_pointer_cast<ReadBackBuffer>(shared_from_this());
        ctx->byteOffset = byteOffset;
        ctx->byteLength = byteLength;
        ctx->dst = dst;

        return std::make_shared<ReadTask>(ctx);
    }

    std::shared_ptr<Core::BaseTask> ReadBackBuffer::CreateCopyToBufferTask(
        const std::shared_ptr<Core::Buffer>& dst,
        std::size_t byteLength,
        std::size_t srcByteOffset,
        std::size_t dstByteOffset) const
    {
        const auto context = std::make_shared<CopyBufferTaskContext>();
        context->src = std::dynamic_pointer_cast<Metal::Buffer>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
        context->dst = std::dynamic_pointer_cast<Metal::Buffer>(dst->GetUnderlyingBuffer());
        context->srcByteOffset = srcByteOffset;
        context->dstByteOffset = dstByteOffset;
        context->byteLength = byteLength;

        return std::make_shared<CopyBufferTask>(context);
    }

    std::shared_ptr<Core::BaseTask> ReadBackBuffer::CreateInitializationTask()
    {
        const auto ctx = std::make_shared<InitTaskContext>();
        ctx->byteSize = GetSettings().byteLength;
        ctx->entity = std::dynamic_pointer_cast<Metal::Buffer>(shared_from_this());
        return std::make_shared<InitTask>(ctx);
    }

    std::shared_ptr<DeviceMemoryHeap> ReadBackBuffer::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
    {
        return globalContext->readBackBufferHeap;
    }


    ResidentBuffer::ResidentBuffer(const Settings& settings) : Core::ResidentBuffer(settings), Metal::Buffer(MTLSettings{})
    {
    }

    std::shared_ptr<Core::BaseTask> ResidentBuffer::CreateCopyToBufferTask(
        const std::shared_ptr<Core::Buffer>& dst,
        std::size_t byteLength,
        std::size_t srcByteOffset,
        std::size_t dstByteOffset) const
    {
        const auto context = std::make_shared<CopyBufferTaskContext>();
        context->src = std::dynamic_pointer_cast<Metal::Buffer>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
        context->dst = std::dynamic_pointer_cast<Metal::Buffer>(dst->GetUnderlyingBuffer());
        context->srcByteOffset = srcByteOffset;
        context->dstByteOffset = dstByteOffset;
        context->byteLength = byteLength;

        return std::make_shared<CopyBufferTask>(context);
    }

    std::shared_ptr<Core::BaseTask> ResidentBuffer::CreateInitializationTask()
    {
        const auto ctx = std::make_shared<InitTaskContext>();
        ctx->byteSize = GetSettings().byteLength;
        ctx->entity = std::dynamic_pointer_cast<Metal::Buffer>(shared_from_this());
        return std::make_shared<InitTask>(ctx);
    }

    std::shared_ptr<DeviceMemoryHeap> ResidentBuffer::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
    {
        return globalContext->residentBufferHeap;
    }

    InputAssemblerBuffer::InputAssemblerBuffer(const Core::InputAssemblerBuffer::Settings& settings) : _ia(settings.ia)
    {
        _upload = std::make_shared<UploadBuffer>(settings.base);
        _resident = std::make_shared<ResidentBuffer>(settings.base);
    }

    InputAssemblerBuffer::~InputAssemblerBuffer() = default;

    VertexBuffer::VertexBuffer(const Core::InputAssemblerBuffer::Settings& settings) : Core::VertexBuffer(settings), Metal::InputAssemblerBuffer(settings)
    {
    }

    IndexBuffer::IndexBuffer(const Core::InputAssemblerBuffer::Settings& settings) : Core::IndexBuffer(settings), Metal::InputAssemblerBuffer(settings)
    {
    }

    std::shared_ptr<Core::BaseTask> IndexBuffer::CreateInitializationTask()
    {
        const auto context = std::make_shared<TaskContext>();
        context->entity = std::dynamic_pointer_cast<Metal::InputAssemblerBuffer>(shared_from_this());
        return std::make_shared<InitTask>(context);
    }

    std::shared_ptr<Core::Buffer> IndexBuffer::GetUnderlyingBuffer()
    {
        return _resident;
    }

    std::shared_ptr<Core::BaseTask> IndexBuffer::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
    {
        return _resident->CreateCopyToBufferTask(dst, byteLength, srcByteOffset, dstByteOffset);
    }

    std::shared_ptr<Core::BaseTask> VertexBuffer::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
    {
        return _resident->CreateCopyToBufferTask(dst, byteLength, srcByteOffset, dstByteOffset);
    }

    std::shared_ptr<Core::BaseTask> VertexBuffer::CreateInitializationTask()
    {
        const auto context = std::make_shared<TaskContext>();
        context->entity = std::dynamic_pointer_cast<Metal::InputAssemblerBuffer>(shared_from_this());
        return std::make_shared<InitTask>(context);
    }

    std::shared_ptr<Core::Buffer> VertexBuffer::GetUnderlyingBuffer()
    {
        return _resident;
    }

    InputAssemblerBuffer::InitTask::InitTask(const std::shared_ptr<TaskContext>& context) : Task(context)
    {
    }

    void InputAssemblerBuffer::InitTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::OnScheduled(stream);

        if (const auto tc = GetTaskContext() ; const auto entity = tc->entity)
        {
            stream->Schedule(entity->_upload->CreateInitializationTask());
            stream->Schedule(entity->_resident->CreateInitializationTask());
            stream->Schedule(Core::StreamBarrierTask::kInstance);
            stream->Schedule(entity->_upload->CreateWriteTask(entity->_ia.rawData, entity->_upload->GetSettings().byteLength, 0));
            stream->Schedule(entity->_upload->CopyToBuffer(entity->_resident));
        }
    }

    void InputAssemblerBuffer::InitTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::OnComplete(stream);

        if(const auto tc = GetTaskContext() ; const auto entity = tc->entity)
        {
            entity->_upload.reset();
        }
    }

    UnorderedAccessBuffer::UnorderedAccessBuffer(const Settings& settings) : Core::UnorderedAccessBuffer(settings), Metal::Buffer(MTLSettings{})
    {
    }

    std::shared_ptr<Core::BaseTask> UnorderedAccessBuffer::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
    {
        const auto context = std::make_shared<CopyBufferTaskContext>();
        context->src = std::dynamic_pointer_cast<Metal::Buffer>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
        context->dst = std::dynamic_pointer_cast<Metal::Buffer>(dst->GetUnderlyingBuffer());
        context->srcByteOffset = srcByteOffset;
        context->dstByteOffset = dstByteOffset;
        context->byteLength = byteLength;

        return std::make_shared<CopyBufferTask>(context);
    }

    std::shared_ptr<Core::BaseTask> UnorderedAccessBuffer::CreateInitializationTask()
    {
        const auto ctx = std::make_shared<InitTaskContext>();
        ctx->byteSize = GetSettings().byteLength;
        ctx->entity = std::dynamic_pointer_cast<Metal::Buffer>(shared_from_this());
        return std::make_shared<InitTask>(ctx);
    }

    std::shared_ptr<DeviceMemoryHeap> UnorderedAccessBuffer::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
    {
        return globalContext->residentBufferHeap;
    }


    CounteredUnorderedAccessBuffer::CounteredUnorderedAccessBuffer(const Settings& settings)
: Core::CounteredUnorderedAccessBuffer(settings), Metal::Buffer(MTLSettings{}),
        _counterBuffer(std::make_shared<UnorderedAccessBuffer>(Core::BaseUnorderedAccessBuffer::Settings{
            sizeof(Core::CounteredUnorderedAccessBuffer::CounterValueType), 1, "counter"
        }))
    {
    }

    std::shared_ptr<Core::BaseTask> CounteredUnorderedAccessBuffer::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
    {
        const auto context = std::make_shared<CopyBufferTaskContext>();
        context->src = std::dynamic_pointer_cast<Metal::Buffer>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
        context->dst = std::dynamic_pointer_cast<Metal::Buffer>(dst->GetUnderlyingBuffer());
        context->srcByteOffset = srcByteOffset;
        context->dstByteOffset = dstByteOffset;
        context->byteLength = byteLength;

        return std::make_shared<CopyBufferTask>(context);
    }

    std::shared_ptr<Core::BaseTask> CounteredUnorderedAccessBuffer::CreateInitializationTask()
    {
        const auto ctx = std::make_shared<InitTaskContext>();
        ctx->byteSize = GetSettings().byteLength;
        ctx->entity = std::dynamic_pointer_cast<Metal::Buffer>(shared_from_this());
        
        return std::make_shared<Core::BatchTask>(std::initializer_list<std::shared_ptr<Core::BaseTask>>{
            std::make_shared<InitTask>(ctx),
            _counterBuffer->CreateInitializationTask(),
            CreateResetCounterTask()
        });
    }

    std::shared_ptr<DeviceMemoryHeap> CounteredUnorderedAccessBuffer::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
    {
        return globalContext->residentBufferHeap;
    }

    std::shared_ptr<Core::BaseTask> CounteredUnorderedAccessBuffer::CreateResetCounterTask()
    {
        const auto ctx = std::make_shared<ResetContext>();
        ctx->entity = _counterBuffer;

        return std::make_shared<ResetCounterTaskImpl>(ctx);
    }

    std::shared_ptr<Metal::Buffer> CounteredUnorderedAccessBuffer::GetCounterBuffer() const
    {
        return _counterBuffer;
    }

    std::shared_ptr<Core::BaseTask> CounteredUnorderedAccessBuffer::CreateCopyCounterTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t dstByteOffset)
    {
        return _counterBuffer->CreateCopyToBufferTask(dst, sizeof(Core::CounteredUnorderedAccessBuffer::CounterValueType), 0, dstByteOffset);
    }

    CounteredUnorderedAccessBuffer::ResetCounterTaskImpl::ResetCounterTaskImpl(const std::shared_ptr<ResetContext>& ctx)
        : Task<MMPEngine::Backend::Metal::CounteredUnorderedAccessBuffer::ResetContext>(ctx)
    {
    }

    void CounteredUnorderedAccessBuffer::ResetCounterTaskImpl::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);

        const auto tc = GetTaskContext();
        
        const auto blitEncoder = _specificStreamContext->PopulateCommandsInBuffer()->GetNative()->blitCommandEncoder();
        
        blitEncoder->fillBuffer(tc->entity->GetNative(), NS::Range(0, static_cast<NS::Integer>(tc->entity->GetSettings().byteLength)), static_cast<std::uint8_t>(0));

        blitEncoder->endEncoding();
    }

}
