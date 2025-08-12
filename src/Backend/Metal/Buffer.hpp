#pragma once
#include <Core/Buffer.hpp>
#include <Backend/Metal/Entity.hpp>
#include <Backend/Metal/Heap.hpp>
#include <Backend/Metal/Context.hpp>

namespace MMPEngine::Backend::Metal
{
    class Buffer : public ResourceEntity
    {
    protected:
        struct MTLSettings final
        {
        };
    public:
        Buffer(const MTLSettings& mtlSettings);
        ~Buffer() override;
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&) noexcept = delete;
        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&&) noexcept = delete;
    protected:

        class InitTaskContext final : public Core::EntityTaskContext<Buffer>
        {
        public:
            std::size_t byteSize = 0;
        };

        class InitTask final : public Task<InitTaskContext>
        {
        public:

            class Allocate final : public Task<InitTaskContext>
            {
            public:
                Allocate(const std::shared_ptr<InitTaskContext>& context);
                void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
            };

            class Create final : public Task<InitTaskContext>
            {
            public:
                Create(const std::shared_ptr<InitTaskContext>& context);
                void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
            };

        public:
            InitTask(const std::shared_ptr<InitTaskContext>& context);
            void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
        };


        class CopyBufferTaskContext : public Core::TaskContext
        {
        public:
            std::shared_ptr<Metal::Buffer> src;
            std::shared_ptr<Metal::Buffer> dst;
            std::size_t byteLength = 0;
            std::size_t srcByteOffset = 0;
            std::size_t dstByteOffset = 0;
        };

        class CopyBufferTask final : public Task<CopyBufferTaskContext>
        {
        public:
            CopyBufferTask(const std::shared_ptr<CopyBufferTaskContext>& context);
            void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
        };

    protected:
        MTL::Buffer* _nativeBuffer = nullptr;
        MTLSettings _mtlSettings;
    };

    class UploadBuffer final : public Core::UploadBuffer, public Buffer
    {
    private:
        class WriteTaskContext final : public Core::UploadBuffer::WriteTaskContext
        {
        public:
            std::shared_ptr<UploadBuffer> uploadBuffer;
        };
        class WriteTask final : public Task<Core::UploadBuffer::WriteTaskContext>
        {
        private:
            class Impl final : public Task<WriteTaskContext>
            {
            public:
                Impl(const std::shared_ptr<WriteTaskContext>& context);
                void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
            };
            std::shared_ptr<BaseTask> _implTask;

        public:
            WriteTask(const std::shared_ptr<WriteTaskContext>& context);
            void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
        };
    public:
        UploadBuffer(const Settings& settings);
        std::shared_ptr<Core::ContextualTask<Core::UploadBuffer::WriteTaskContext>> CreateWriteTask(const void* src, std::size_t byteLength, std::size_t byteOffset) override;
        std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
    protected:
        std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
    };

    class ReadBackBuffer final : public Core::ReadBackBuffer, public Buffer
    {
    private:
        class ReadTaskContext final : public Core::ReadBackBuffer::ReadTaskContext
        {
        public:
            std::shared_ptr<ReadBackBuffer> readBackBuffer;
        };
        class ReadTask final : public Task<Core::ReadBackBuffer::ReadTaskContext>
        {
        private:
            class Impl final : public Task<ReadTaskContext>
            {
            public:
                Impl(const std::shared_ptr<ReadTaskContext>& context);
                void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
            };
            std::shared_ptr<BaseTask> _implTask;
        public:
            ReadTask(const std::shared_ptr<ReadTaskContext>& context);
            void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
        };
    public:
        ReadBackBuffer(const Settings& settings);
        std::shared_ptr<Core::ContextualTask<Core::ReadBackBuffer::ReadTaskContext>> CreateReadTask(void* dst, std::size_t byteLength, std::size_t byteOffset) override;
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
        std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
    protected:
        std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
    };

    class ResidentBuffer : public Core::ResidentBuffer, public Buffer
    {
    public:
        ResidentBuffer(const Settings& settings);
        std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
    protected:
        std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
    };

    template<class TUniformBufferData>
    class UniformBuffer final : public Core::UniformBuffer<TUniformBufferData>, public Metal::Buffer
    {
    private:
        class WriteTaskContext final : public Core::UniformBuffer<TUniformBufferData>::WriteTaskContext
        {
        public:
            std::shared_ptr<UniformBuffer> uniformBuffer;
        };

        class WriteTask final : public Task<typename Core::UniformBuffer<TUniformBufferData>::WriteTaskContext>
        {
        private:
            class Impl final : public Task<WriteTaskContext>
            {
            public:
                Impl(const std::shared_ptr<WriteTaskContext>& context);
                void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
            };
            std::shared_ptr<Core::BaseTask> _implTask;
        public:
            WriteTask(const std::shared_ptr<WriteTaskContext>& ctx);
        protected:
            void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
        };
    public:
        UniformBuffer(std::string_view name);
        UniformBuffer();
        std::shared_ptr<Core::ContextualTask<typename Core::UniformBuffer<TUniformBufferData>::WriteTaskContext>> CreateWriteAsyncTask(const TUniformBufferData& data) override;
        std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
    protected:
        std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;

    };

    template<class TUniformBufferData>
    inline UniformBuffer<TUniformBufferData>::UniformBuffer(std::string_view name)
        : Core::UniformBuffer<TUniformBufferData>(Core::Buffer::Settings {sizeof(typename UniformBuffer<TUniformBufferData>::TData), std::string {name}}),
        Metal::Buffer(MTLSettings {})
    {
    }

    template<class TUniformBufferData>
    inline UniformBuffer<TUniformBufferData>::UniformBuffer()
        : Core::UniformBuffer<TUniformBufferData>(Core::Buffer::Settings {sizeof(typename UniformBuffer<TUniformBufferData>::TData), std::string {}}),
        Metal::Buffer(MTLSettings{})
    {
    }

    template <class TUniformBufferData>
    std::shared_ptr<DeviceMemoryHeap> UniformBuffer<TUniformBufferData>::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
    {
        return globalContext->uniformBufferHeap;
    }

    template <class TUniformBufferData>
    std::shared_ptr<Core::BaseTask> UniformBuffer<TUniformBufferData>::CreateInitializationTask()
    {
        const auto ctx = std::make_shared<InitTaskContext>();
        ctx->byteSize = this->GetSettings().byteLength;
        ctx->entity = std::dynamic_pointer_cast<Metal::Buffer>(shared_from_this());
        return std::make_shared<InitTask>(ctx);
    }

    template <class TUniformBufferData>
    std::shared_ptr<Core::BaseTask> UniformBuffer<TUniformBufferData>::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
    {
        const auto context = std::make_shared<CopyBufferTaskContext>();
        context->src = std::dynamic_pointer_cast<Metal::Buffer>(std::const_pointer_cast<Core::Buffer>(this->GetUnderlyingBuffer()));
        context->dst = std::dynamic_pointer_cast<Metal::Buffer>(dst->GetUnderlyingBuffer());
        context->srcByteOffset = srcByteOffset;
        context->dstByteOffset = dstByteOffset;
        context->byteLength = byteLength;

        return std::make_shared<CopyBufferTask>(context);
    }

    template <class TUniformBufferData>
    UniformBuffer<TUniformBufferData>::WriteTask::Impl::Impl(const std::shared_ptr<WriteTaskContext>& context)
        : Task<UniformBuffer<TUniformBufferData>::WriteTaskContext>(context)
    {
    }

    template <class TUniformBufferData>
    void UniformBuffer<TUniformBufferData>::WriteTask::Impl::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task<UniformBuffer<TUniformBufferData>::WriteTaskContext>::Run(stream);

        if (const auto tc = this->GetTaskContext(); const auto entity = tc->uniformBuffer)
        {
            std::memcpy(static_cast<char*>(entity->_nativeBuffer->contents()), std::addressof(tc->data), sizeof(tc->data));
        }
    }


    template <class TUniformBufferData>
    UniformBuffer<TUniformBufferData>::WriteTask::WriteTask(const std::shared_ptr<WriteTaskContext>& ctx)
        : Task<typename Core::UniformBuffer<TUniformBufferData>::WriteTaskContext>(ctx)
    {
        _implTask = std::make_shared<Impl>(ctx);
    }

    template <class TUniformBufferData>
    void UniformBuffer<TUniformBufferData>::WriteTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task<typename Core::UniformBuffer<TUniformBufferData>::WriteTaskContext>::OnScheduled(stream);

        stream->Schedule(Core::StreamBarrierTask::kInstance);
        stream->Schedule(_implTask);
    }


    template <class TUniformBufferData>
    std::shared_ptr<Core::ContextualTask<typename Core::UniformBuffer<TUniformBufferData>::WriteTaskContext>> UniformBuffer<TUniformBufferData>::CreateWriteAsyncTask(const TUniformBufferData& data)
    {
        const auto ctx = std::make_shared<WriteTaskContext>();
        ctx->uniformBuffer = std::dynamic_pointer_cast<UniformBuffer>(this->shared_from_this());
        std::memcpy(std::addressof(ctx->data), std::addressof(data), sizeof(data));
        return std::make_shared<WriteTask>(ctx);
    }

}
