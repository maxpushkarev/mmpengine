#include <Backend/Metal/Buffer.hpp>
#include <cassert>

namespace MMPEngine::Backend::Metal
{
    Buffer::Buffer(const MTLSettings& mtlSettings) : _mtlSettings(mtlSettings)
    {
    }

    Buffer::~Buffer()
    {
        if(_nativeBuffer)
        {
            _nativeBuffer->release();
        }
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
        
        tc->entity->_nativeBuffer = tc->entity->_deviceMemoryHeapHandle.GetMemoryBlock()->GetNative()->newBuffer(static_cast<NS::UInteger>(tc->entity->_deviceMemoryHeapHandle.GetSize()), memHeap->GetMtlSettings().resourceOption, static_cast<NS::UInteger>(tc->entity->_deviceMemoryHeapHandle.GetOffset()));
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
            srcBuffer->_nativeBuffer,
            static_cast<NS::UInteger>(tc->srcByteOffset),
            dstBuffer->_nativeBuffer,
            static_cast<NS::UInteger>(tc->dstByteOffset),
            static_cast<NS::UInteger>(tc->byteLength)
        );
        
        blitEncoder->endEncoding();
        blitEncoder->release();
        
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

}
