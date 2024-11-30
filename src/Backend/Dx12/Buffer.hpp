#pragma once
#include <Core/Buffer.hpp>
#include <Backend/Dx12/Entity.hpp>

namespace MMPEngine::Backend::Dx12
{
	class CopyBufferTaskContext : public Core::TaskContext
	{
	public:
		std::weak_ptr<Dx12::BaseEntity> src;
		std::weak_ptr<Dx12::BaseEntity> dst;
		std::size_t byteLength = 0;
		std::size_t srcByteOffset = 0;
		std::size_t dstByteOffset = 0;
	};

	class CopyBufferTask final : public Task, public Core::TaskWithInternalContext<CopyBufferTaskContext>
	{
	public:
		CopyBufferTask(const std::shared_ptr<CopyBufferTaskContext>& context);
		void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		void Finalize(const std::shared_ptr<Core::BaseStream>& stream) override;
	private:
		std::shared_ptr<BaseTask> _switchSrcStateTask;
		std::shared_ptr<BaseTask> _switchDstStateTask;
	};

	class MappedBuffer : public ResourceEntity
	{
	public:
		MappedBuffer(std::string_view name);
		MappedBuffer();
		MappedBuffer(const MappedBuffer&) = delete;
		MappedBuffer(MappedBuffer&&) noexcept = delete;
		MappedBuffer& operator=(const MappedBuffer&) = delete;
		MappedBuffer& operator=(MappedBuffer&&) noexcept = delete;
		~MappedBuffer() override;
	protected:

		class InitTaskContext final : public InitContext<MappedBuffer>
		{
		public:
			D3D12_HEAP_TYPE heapType;
			std::size_t byteSize;
		};

		class InitTask final : public Task, public Core::TaskWithInternalContext<InitTaskContext>
		{
		public:
			InitTask(const std::shared_ptr<InitTaskContext>& context);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			void Finalize(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		void Map();
		void Unmap();
		void* _mappedBufferPtr = nullptr;
	};

	class UploadBuffer final : public Core::UploadBuffer, public MappedBuffer
	{
	public:
		UploadBuffer(const Settings& settings);
		void Write(const void* src, std::size_t byteLength, std::size_t byteOffset) override;
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	};

	class ReadBackBuffer final : public Core::ReadBackBuffer, public MappedBuffer
	{
	public:
		ReadBackBuffer(const Settings& settings);
		void Read(void* dst, std::size_t byteLength, std::size_t byteOffset) override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) override;
	};

}