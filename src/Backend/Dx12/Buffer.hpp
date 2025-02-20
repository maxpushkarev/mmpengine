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
			bool withCounter = false;
			Core::BaseUnorderedAccessBuffer::Settings settings{};
		};
		class InitUaTask final : public Task<InitUaTaskContext>
		{
		public:
			InitUaTask(const std::shared_ptr<InitUaTaskContext>& context);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

	public:
		const BaseDescriptorPool::Handle* GetResourceDescriptorHandle() const override;

		const BaseDescriptorPool::Handle* GetShaderInVisibleCounterDescriptorHandle() const;
		const BaseDescriptorPool::Handle* GetShaderVisibleCounterDescriptorHandle() const;

	private:
		BaseDescriptorPool::Handle _shaderVisibleHandle;
		BaseDescriptorPool::Handle _shaderVisibleHandleCounter;
		BaseDescriptorPool::Handle _shaderInVisibleHandleCounter;
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
		class ResetCounterTaskContext final : public Core::EntityTaskContext<UaBuffer>
		{
		public:
			Core::BaseUnorderedAccessBuffer::Settings settings;
		};
		class ResetCounter final : public Task<ResetCounterTaskContext>
		{
		private:
			class Impl final : public Task<ResetCounterTaskContext>
			{
			public:
				Impl(const std::shared_ptr<ResetCounterTaskContext>& ctx);
			protected:
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};
		public:
			ResetCounter(const std::shared_ptr<ResetCounterTaskContext>& ctx);
		protected:
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		private:
			std::shared_ptr<BindDescriptorPoolsTask> _bindDescriptorPoolsTask;
			std::shared_ptr<Impl> _impl;
			std::shared_ptr<Core::BaseTask> _switchStateTask;
		};

	public:
		CounteredUnorderedAccessBuffer(const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateResetCounterTask() override;
		std::shared_ptr<Core::BaseTask> CreateCopyCounterTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t dstByteOffset) override;
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

	template<class TUniformBufferData>
	class UniformBuffer final : public Core::UniformBuffer<TUniformBufferData>, public ResourceEntity
	{
	public:
		UniformBuffer(std::string_view name);
		UniformBuffer();

		std::shared_ptr<Core::ContextualTask<typename Core::UniformBuffer<TUniformBufferData>::WriteTaskContext>> CreateWriteAsyncTask(const TUniformBufferData& data) override;
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateSwitchStateTask(D3D12_RESOURCE_STATES nextStateMask) override;
		const BaseDescriptorPool::Handle* GetResourceDescriptorHandle() const override;

	private:

		class WriteTaskContext final : public Core::UniformBuffer<TUniformBufferData>::WriteTaskContext
		{
		public:
			std::shared_ptr<UniformBuffer> uniformBuffer;
		};

		class WriteTask final : public Task<typename Core::UniformBuffer<TUniformBufferData>::WriteTaskContext>
		{
		public:
			WriteTask(const std::shared_ptr<WriteTaskContext>& ctx);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		private:
			std::shared_ptr<Core::ContextualTask<Core::UploadBuffer::WriteTaskContext>> _impl;
		};

		class InitTaskContext final : public Core::EntityTaskContext<UniformBuffer>
		{
		};

		class InitTask final : public Task<InitTaskContext>
		{
		private:

			class Alloc final : public Task<InitTaskContext>
			{
			public:
				Alloc(const std::shared_ptr<InitTaskContext>& context);
				void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

			class CreateDescriptors final : public Task<InitTaskContext>
			{
			public:
				CreateDescriptors(const std::shared_ptr<InitTaskContext>& context);
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

		public:
			InitTask(const std::shared_ptr<InitTaskContext>& context);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		private:
			std::shared_ptr<Core::BaseTask> _alloc;
			std::shared_ptr<Core::BaseTask> _createDescriptors;
		};

		static std::size_t GetRequiredSize();
		ConstantBufferHeap::Handle _cbHeapHandle;
		BaseDescriptorPool::Handle _descriptorHeapHandle;
	};

	template<class TUniformBufferData>
	inline UniformBuffer<TUniformBufferData>::UniformBuffer(std::string_view name) : Core::UniformBuffer<TUniformBufferData>(Core::Buffer::Settings {GetRequiredSize(), std::string {name}})
	{
	}

	template<class TUniformBufferData>
	inline UniformBuffer<TUniformBufferData>::UniformBuffer() : Core::UniformBuffer<TUniformBufferData>(Core::Buffer::Settings {GetRequiredSize(), ""})
	{
	}

	template <class TUniformBufferData>
	std::shared_ptr<Core::ContextualTask<typename Core::UniformBuffer<TUniformBufferData>::WriteTaskContext>> UniformBuffer<TUniformBufferData>::CreateWriteAsyncTask(const TUniformBufferData& data)
	{
		const auto ctx = std::make_shared<WriteTaskContext>();
		ctx->uniformBuffer = std::dynamic_pointer_cast<UniformBuffer>(this->shared_from_this());
		std::memcpy(std::addressof(ctx->data), std::addressof(data), sizeof(data));
		return std::make_shared<WriteTask>(ctx);
	}

	template<class TUniformBufferData>
	inline std::shared_ptr<Core::BaseTask> UniformBuffer<TUniformBufferData>::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<UniformBuffer>(this->shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	template<class TUniformBufferData>
	std::shared_ptr<Core::BaseTask> UniformBuffer<TUniformBufferData>::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		const auto globalOffset = this->_cbHeapHandle.GetOffset();
		return this->_cbHeapHandle.GetUploadBlock()->CreateCopyToBufferTask(
			dst->GetUnderlyingBuffer(),
			byteLength, 
			globalOffset + srcByteOffset, 
			dstByteOffset
		);
	}

	template<class TUniformBufferData>
	inline std::shared_ptr<Core::BaseTask> UniformBuffer<TUniformBufferData>::CreateSwitchStateTask(D3D12_RESOURCE_STATES nextStateMask)
	{
		return this->_cbHeapHandle.GetUploadBlock()->CreateSwitchStateTask(nextStateMask);
	}

	template<class TUniformBufferData>
	inline const BaseDescriptorPool::Handle* UniformBuffer<TUniformBufferData>::GetResourceDescriptorHandle() const
	{
		return &(this->_descriptorHeapHandle);
	}

	template<class TUniformBufferData>
	inline std::size_t UniformBuffer<TUniformBufferData>::GetRequiredSize()
	{
		const auto res = (
			(sizeof(Core::UniformBuffer<TUniformBufferData>::TData) + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) /
				D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT * D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT
		);
		return res;
	}

	template <class TUniformBufferData>
	inline UniformBuffer<TUniformBufferData>::WriteTask::WriteTask(const std::shared_ptr<WriteTaskContext>& ctx) : Task<typename Core::UniformBuffer<TUniformBufferData>::WriteTaskContext>(ctx)
	{
		const auto offset = ctx->uniformBuffer->_cbHeapHandle.GetOffset();

		this->_impl = ctx->uniformBuffer->_cbHeapHandle.GetUploadBlock()->CreateWriteTask(
			std::addressof(this->GetTaskContext()->data),
			sizeof(typename Core::UniformBuffer<TUniformBufferData>::TData),
			offset
		);
	}

	template<class TUniformBufferData>
	inline void UniformBuffer<TUniformBufferData>::WriteTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task<typename Core::UniformBuffer<TUniformBufferData>::WriteTaskContext>::OnScheduled(stream);
		stream->Schedule(this->_impl);
	}

	template <class TUniformBufferData>
	inline UniformBuffer<TUniformBufferData>::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& context) : Task<InitTaskContext>(context)
	{
		_alloc = std::make_shared<Alloc>(context);
		_createDescriptors = std::make_shared<CreateDescriptors>(context);
	}

	template <class TUniformBufferData>
	inline void UniformBuffer<TUniformBufferData>::InitTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task<InitTaskContext>::OnScheduled(stream);

		stream->Schedule(this->_alloc),
		stream->Schedule(this->_specificGlobalContext->constantBufferEntityHeap->CreateTaskToInitializeBlocks());
		stream->Schedule(this->_createDescriptors);
	}

	template <class TUniformBufferData>
	inline UniformBuffer<TUniformBufferData>::InitTask::Alloc::Alloc(const std::shared_ptr<InitTaskContext>& context) : Task<InitTaskContext>(context)
	{
	}

	template <class TUniformBufferData>
	inline void UniformBuffer<TUniformBufferData>::InitTask::Alloc::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task<InitTaskContext>::OnScheduled(stream);

		const auto ctx = this->GetTaskContext();

		ctx->entity->_cbHeapHandle = this->_specificGlobalContext->constantBufferEntityHeap->Allocate(Core::Heap::Request {
			ctx->entity->GetRequiredSize(),
				D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT
		});
	}

	template <class TUniformBufferData>
	inline UniformBuffer<TUniformBufferData>::InitTask::CreateDescriptors::CreateDescriptors(const std::shared_ptr<InitTaskContext>& context) : Task<InitTaskContext>(context)
	{
	}

	template <class TUniformBufferData>
	inline void UniformBuffer<TUniformBufferData>::InitTask::CreateDescriptors::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task<InitTaskContext>::Run(stream);

		const auto tc = this->GetTaskContext();
		const auto gc = this->_specificGlobalContext;

		tc->entity->SetNativeResource(
			tc->entity->_cbHeapHandle.GetUploadBlock()->GetNativeResource(), static_cast<std::uint32_t>(tc->entity->_cbHeapHandle.GetOffset())
		);

		tc->entity->_descriptorHeapHandle = gc->cbvSrvUavShaderVisibleDescPool->Allocate();

		D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc{};
		viewDesc.BufferLocation = tc->entity->GetNativeResource()->GetGPUVirtualAddress() + static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(tc->entity->_cbHeapHandle.GetOffset());
		viewDesc.SizeInBytes = static_cast<std::uint32_t>(tc->entity->GetSettings().byteLength);

		gc->device->CreateConstantBufferView(&viewDesc, tc->entity->_descriptorHeapHandle.GetCPUDescriptorHandle());
	}

}
