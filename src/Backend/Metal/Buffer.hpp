#pragma once
#include <Core/Buffer.hpp>
#include <Backend/Metal/Entity.hpp>
#include <Backend/Metal/Context.hpp>
#include <Backend/Metal/Task.hpp>
#include <Metal/Metal.hpp>

namespace MMPEngine::Backend::Metal
{
    class Buffer : public ResourceEntity
    {
    protected:
        struct MtlSettings final
        {
            
        };
    public:
        Buffer(const MtlSettings&);
        ~Buffer() override;
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&) noexcept = delete;
        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&&) noexcept = delete;
    protected:
        
        MtlSettings _mtlSettings;
        std::shared_ptr<GlobalContext> _globalContext;
        MTL::Buffer* _nativeBuffer = nullptr;
        
        class InitTaskContext final : public Core::EntityTaskContext<Buffer>
        {
        public:
            std::size_t byteSize = 0;
        };

        class InitTask final : public Task<InitTaskContext>
        {
        public:
            InitTask(const std::shared_ptr<InitTaskContext>& context);
            void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
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
    };

    class ResidentBuffer : public Core::ResidentBuffer, public Buffer
    {
    public:
        ResidentBuffer(const Settings& settings);
        std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
    };
}
