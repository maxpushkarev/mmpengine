#pragma once
#include <Core/Buffer.hpp>
#include <Backend/Dx12/Entity.hpp>
#include <Core/Context.hpp>

namespace MMPEngine::Backend::Dx12
{
	class CopyBufferTaskContext : public Core::TaskContext
	{
	public:
		std::shared_ptr<Dx12::BaseEntity> src;
		std::shared_ptr<Dx12::BaseEntity> dst;
		std::size_t byteLength = 0;
		std::size_t srcByteOffset = 0;
		std::size_t dstByteOffset = 0;
	};

	class CopyBufferTask final : public Task<CopyBufferTaskContext>
	{
	private:
		class Impl final : public Task<CopyBufferTaskContext>
		{
		public:
			Impl(const std::shared_ptr<CopyBufferTaskContext>& context);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	public:
		CopyBufferTask(const std::shared_ptr<CopyBufferTaskContext>& context);
		void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
	private:
		std::shared_ptr<BaseTask> _switchSrcStateTask;
		std::shared_ptr<BaseTask> _switchDstStateTask;
		std::shared_ptr<Impl> _commandTask;
	};

	class Buffer : public ResourceEntity
	{
	public:
		Buffer(std::string_view name);
		Buffer();

		class InitTaskContext final : public Core::EntityTaskContext<Buffer>
		{
		public:
			D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT;
			bool unorderedAccess = false;
			std::size_t byteSize = 0;
		};

		class CreateBufferTask : public Task<InitTaskContext>
		{
		public:
			CreateBufferTask(const std::shared_ptr<InitTaskContext>& context);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class InitTask final : public Task<InitTaskContext>
		{
		public:
			InitTask(const std::shared_ptr<InitTaskContext>& context);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	};


	class MappedBuffer : public Buffer
	{
		friend class MMPEngine::Backend::Dx12::Buffer::CreateBufferTask;

	public:
		MappedBuffer(std::string_view name);
		MappedBuffer();
		MappedBuffer(const MappedBuffer&) = delete;
		MappedBuffer(MappedBuffer&&) noexcept = delete;
		MappedBuffer& operator=(const MappedBuffer&) = delete;
		MappedBuffer& operator=(MappedBuffer&&) noexcept = delete;
		~MappedBuffer() override;
	protected:

		void Map();
		void Unmap();
		void* _mappedBufferPtr = nullptr;
		
		class MappedBufferTask
		{
		protected:
			static void* GetMappedPtr(const std::shared_ptr<MappedBuffer>& mappedBuffer);
		};
	};


	class ResidentBuffer : public Core::ResidentBuffer, public Buffer
	{
	public:
		ResidentBuffer(const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	};

	class UaBuffer : public Buffer
	{
	protected:
		class InitUaTaskContext : public Core::EntityTaskContext<UaBuffer>
		{
		public:
			bool isCounter = false;
			std::shared_ptr<Dx12::UaBuffer> counterAttachment = nullptr;
			Core::BaseUnorderedAccessBuffer::Settings settings{};
		};

		class CreateUaDescriptors final : public Task<InitUaTaskContext>
		{
		public:
			CreateUaDescriptors(const std::shared_ptr<InitUaTaskContext>& context);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class InitUaTask final : public Task<InitUaTaskContext>
		{
		public:
			InitUaTask(const std::shared_ptr<InitUaTaskContext>& context);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

	public:
		const BaseDescriptorHeap::Handle* GetShaderInVisibleDescriptorHandle() const override;
		const BaseDescriptorHeap::Handle* GetShaderVisibleDescriptorHandle() const override;
	private:
		BaseDescriptorHeap::Handle _shaderVisibleHandle;
		BaseDescriptorHeap::Handle _shaderInVisibleHandle;
	};

	class UnorderedAccessBuffer final : public Core::UnorderedAccessBuffer, public UaBuffer
	{
	public:
		UnorderedAccessBuffer(const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	};

	class CounteredUnorderedAccessBuffer final : public Core::CounteredUnorderedAccessBuffer, public UaBuffer
	{
	private:
		class ResetCounter final : public Task<Core::EntityTaskContext<UaBuffer>>
		{
		private:
			class Impl final : public Task<Core::EntityTaskContext<UaBuffer>>
			{
			public:
				Impl(const std::shared_ptr<Core::EntityTaskContext<UaBuffer>>& ctx);
			protected:
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};
		public:
			ResetCounter(const std::shared_ptr<Core::EntityTaskContext<UaBuffer>>& ctx);
		protected:
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

	public:
		CounteredUnorderedAccessBuffer(const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateSwitchStateTask(D3D12_RESOURCE_STATES nextStateMask) override;
		std::shared_ptr<Core::BaseTask> CreateResetCounterTask() override;
		std::shared_ptr<Core::BaseTask> CreateCopyCounterTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t dstByteOffset) override;
	private:
		//TODO: try to merge data with counter (usage CounterOffsetInBytes)
		std::shared_ptr<UnorderedAccessBuffer> _counter;
	};

	class UploadBuffer final : public Core::UploadBuffer, public MappedBuffer
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
			class Impl final : public MappedBufferTask, public Task<WriteTaskContext>
			{
			public:
				Impl(const std::shared_ptr<WriteTaskContext>& context);
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};
			std::shared_ptr<BaseTask> _prepareStateTask;
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

	class ReadBackBuffer final : public Core::ReadBackBuffer, public MappedBuffer
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
			class Impl final : public MappedBufferTask, public Task<ReadTaskContext>
			{
			public:
				Impl(const std::shared_ptr<ReadTaskContext>& context);
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};
			std::shared_ptr<BaseTask> _prepareStateTask;
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

	class InputAssemblerBuffer
	{
	public:
		InputAssemblerBuffer(const InputAssemblerBuffer&) = delete;
		InputAssemblerBuffer(InputAssemblerBuffer&&) noexcept = delete;
		InputAssemblerBuffer& operator=(const InputAssemblerBuffer&) = delete;
		InputAssemblerBuffer& operator=(InputAssemblerBuffer&&) noexcept = delete;
	protected:
		InputAssemblerBuffer(const Core::InputAssemblerBuffer::Settings& settings);
		virtual ~InputAssemblerBuffer();

		class TaskContext final : public Core::EntityTaskContext<InputAssemblerBuffer>
		{
		};

		class InitTask final : public Task<TaskContext>
		{
		public:
			InitTask(const std::shared_ptr<TaskContext>& context);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			void OnComplete(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	protected:
		std::shared_ptr<UploadBuffer> _upload;
		std::shared_ptr<ResidentBuffer> _resident;
	private:
		Core::InputAssemblerBuffer::IASettings _ia;
	};

	class VertexBuffer final : public Core::VertexBuffer, public Dx12::InputAssemblerBuffer
	{
	public:
		VertexBuffer(const Core::InputAssemblerBuffer::Settings& settings);
		std::shared_ptr<Core::Buffer> GetUnderlyingBuffer() override;
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	};

	class IndexBuffer final : public Core::IndexBuffer, public Dx12::InputAssemblerBuffer
	{
	public:
		IndexBuffer(const Core::InputAssemblerBuffer::Settings& settings);
		std::shared_ptr<Core::Buffer> GetUnderlyingBuffer() override;
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	};

	template<class TConstantBufferData>
	class ConstantBuffer final : public Core::ConstantBuffer<TConstantBufferData>
	{
	public:
		ConstantBuffer(std::string_view name);
		ConstantBuffer();
		std::shared_ptr<Core::ContextualTask<Core::UploadBuffer::WriteTaskContext>> CreateWriteAsyncTask(const TConstantBufferData& data) override;

		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::Buffer> GetUnderlyingBuffer() override;

	private:
		static std::size_t GetRequiredSize();
		std::shared_ptr<UploadBuffer> _uploadBuffer;
	};

	template<class TConstantBufferData>
	inline ConstantBuffer<TConstantBufferData>::ConstantBuffer(std::string_view name) : Core::ConstantBuffer<TConstantBufferData>(Core::Buffer::Settings {GetRequiredSize(), std::string {name}})
	{
		_uploadBuffer = std::make_shared<UploadBuffer>(this->_settings);
	}

	template<class TConstantBufferData>
	inline ConstantBuffer<TConstantBufferData>::ConstantBuffer() : Core::ConstantBuffer<TConstantBufferData>(Core::Buffer::Settings {GetRequiredSize(), ""})
	{
		_uploadBuffer = std::make_shared<UploadBuffer>(this->_settings);
	}

	template <class TConstantBufferData>
	std::shared_ptr<Core::ContextualTask<Core::UploadBuffer::WriteTaskContext>> ConstantBuffer<TConstantBufferData>::CreateWriteAsyncTask(const TConstantBufferData& data)
	{
		return _uploadBuffer->CreateWriteTask(std::addressof(data), sizeof(data), 0);
	}

	template<class TConstantBufferData>
	inline std::shared_ptr<Core::BaseTask> ConstantBuffer<TConstantBufferData>::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		return _uploadBuffer->CreateCopyToBufferTask(dst->GetUnderlyingBuffer(), byteLength, srcByteOffset, dstByteOffset);
	}

	template<class TConstantBufferData>
	inline std::shared_ptr<Core::BaseTask> ConstantBuffer<TConstantBufferData>::CreateInitializationTask()
	{
		return _uploadBuffer->CreateInitializationTask();
	}

	template<class TConstantBufferData>
	inline std::shared_ptr<Core::Buffer> ConstantBuffer<TConstantBufferData>::GetUnderlyingBuffer()
	{
		return _uploadBuffer;
	}

	template<class TConstantBufferData>
	inline std::size_t ConstantBuffer<TConstantBufferData>::GetRequiredSize()
	{
		// Constant buffers must be a multiple of the minimum hardware
		// allocation size (usually 256 bytes).  So round up to nearest
		// multiple of 256.  We do this by adding 255 and then masking off
		// the lower 2 bytes which store all bits < 256.
		// Example: Suppose byteSize = 300.
		// (300 + 255) & ~255
		// 555 & ~255
		// 0x022B & ~0x00ff
		// 0x022B & 0xff00
		// 0x0200
		// 512
		return (sizeof(TConstantBufferData) + 255) & ~255;
	}

}
