#include <Backend/Metal/Buffer.hpp>
#include <cassert>

namespace MMPEngine::Backend::Metal
{
    Buffer::Buffer()
    {
    }

    Buffer::~Buffer()
    {
        if(_nativeBuffer)
        {
            _nativeBuffer->release();
        }
    }

    Buffer::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& context) : Task<MMPEngine::Backend::Metal::Buffer::InitTaskContext>(context)
    {
    }

    void Buffer::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);
        
        const auto buffer = GetTaskContext()->entity;
        buffer->_globalContext = _specificGlobalContext;
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

       
    }

    UploadBuffer::UploadBuffer(const Settings& settings) : Core::UploadBuffer(settings), Metal::Buffer()
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

    ReadBackBuffer::ReadBackBuffer(const Settings& settings) : Core::ReadBackBuffer(settings), Metal::Buffer()
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

    ResidentBuffer::ResidentBuffer(const Settings& settings) : Core::ResidentBuffer(settings), Metal::Buffer()
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
}
