#pragma once
#include <Core/Buffer.hpp>
#include <Backend/Vulkan/Entity.hpp>
#include <Backend/Vulkan/Context.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class Buffer : public ResourceEntity
	{
	public:
		Buffer(VkBufferUsageFlags usage);
		~Buffer() override;
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) noexcept = delete;
		Buffer& operator=(const Buffer&) = delete;
		Buffer& operator=(Buffer&&) noexcept = delete;

		const VkDescriptorBufferInfo& GetDescriptorBufferInfo() const;
		virtual std::shared_ptr<Core::BaseTask> CreateMemoryBarrierTask(VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	protected:

		class MemoryBarrierContext final : public Core::EntityTaskContext<Buffer>
		{
		public:
			VkAccessFlags srcAccess = VK_ACCESS_MEMORY_READ_BIT;
			VkAccessFlags dstAccess = VK_ACCESS_MEMORY_WRITE_BIT;
			VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		};

		class MemoryBarrierTask final : public Task<MemoryBarrierContext>
		{
		public:
			MemoryBarrierTask(const std::shared_ptr<MemoryBarrierContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class InitTaskContext final : public Core::EntityTaskContext<Buffer>
		{
		public:
			std::size_t byteSize = 0;
		};

		class InitTask final : public Task<InitTaskContext>
		{
		public:

			class Create final : public Task<InitTaskContext>
			{
			public:
				Create(const std::shared_ptr<InitTaskContext>& context);
				void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

			class Bind final : public Task<InitTaskContext>
			{
			public:
				Bind(const std::shared_ptr<InitTaskContext>& context);
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

		public:
			InitTask(const std::shared_ptr<InitTaskContext>& context);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		};


		class CopyBufferTaskContext : public Core::TaskContext
		{
		public:
			std::shared_ptr<Vulkan::Buffer> src;
			std::shared_ptr<Vulkan::Buffer> dst;
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
			std::shared_ptr<BaseTask> _srcBufferBarrierTask;
			std::shared_ptr<BaseTask> _dstBufferBarrierTask;
			std::shared_ptr<Impl> _commandTask;
		};

	protected:
		VkBuffer _nativeBuffer = VK_NULL_HANDLE;
		std::shared_ptr<Wrapper::Device> _device;
		VkBufferUsageFlags _usage;
		VkDescriptorBufferInfo _info;
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

	class UnorderedAccessBuffer final : public Core::UnorderedAccessBuffer, public Buffer
	{
	public:
		UnorderedAccessBuffer(const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	protected:
		std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
	};

	class CounteredUnorderedAccessBuffer final : public Core::CounteredUnorderedAccessBuffer, public Buffer
	{
	public:
		CounteredUnorderedAccessBuffer(const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateCopyCounterTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t dstByteOffset) override;
		std::shared_ptr<Core::BaseTask> CreateResetCounterTask() override;
		std::shared_ptr<Core::BaseTask> CreateMemoryBarrierTask(VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage) override;
		std::shared_ptr<Vulkan::Buffer> GetCounterBuffer() const;
	protected:
		std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
	private:
		std::shared_ptr<UnorderedAccessBuffer> _counterBuffer;

		class ResetContext final : public Core::EntityTaskContext<UnorderedAccessBuffer>
		{
		};

		class ResetCounterTaskImpl final : public Task<ResetContext>
		{
		public:
			ResetCounterTaskImpl(const std::shared_ptr<ResetContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	};


	class InputAssemblerBuffer : public Vulkan::Buffer
	{
	public:
		InputAssemblerBuffer(const InputAssemblerBuffer&) = delete;
		InputAssemblerBuffer(InputAssemblerBuffer&&) noexcept = delete;
		InputAssemblerBuffer& operator=(const InputAssemblerBuffer&) = delete;
		InputAssemblerBuffer& operator=(InputAssemblerBuffer&&) noexcept = delete;
		std::shared_ptr<Core::BaseTask> CreateMemoryBarrierTask(VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) override;
	protected:
		InputAssemblerBuffer(VkBufferUsageFlags usage, const Core::InputAssemblerBuffer::Settings& settings, const std::shared_ptr<UploadBuffer>& upload, const std::shared_ptr<Core::Buffer>& storage);
		std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
		~InputAssemblerBuffer() override;

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
		std::shared_ptr<Core::Buffer> _storage;
	private:
		Core::InputAssemblerBuffer::IASettings _ia;
	};

	class VertexBuffer final : public Core::VertexBuffer, public Vulkan::InputAssemblerBuffer
	{
	private:
		class Internal : public Core::ResidentBuffer, public Vulkan::Buffer
		{
		public:
			Internal(const Settings& settings);
			std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
			std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		protected:
			std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
		};
	public:
		VertexBuffer(const Core::InputAssemblerBuffer::Settings& settings);
		std::shared_ptr<Core::Buffer> GetUnderlyingBuffer() override;
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	};

	class IndexBuffer final : public Core::IndexBuffer, public Vulkan::InputAssemblerBuffer
	{
	private:
		class Internal : public Core::ResidentBuffer, public Vulkan::Buffer
		{
		public:
			Internal(const Settings& settings);
			std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
			std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		protected:
			std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
		};
	public:
		IndexBuffer(const Core::InputAssemblerBuffer::Settings& settings);
		std::shared_ptr<Core::Buffer> GetUnderlyingBuffer() override;
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	};

	template<class TUniformBufferData>
	class UniformBuffer final : public Core::UniformBuffer<TUniformBufferData>, public Vulkan::Buffer
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
		: Core::UniformBuffer<TUniformBufferData>(Core::Buffer::Settings {sizeof(UniformBuffer<TUniformBufferData>::TData), std::string {name}}),
		Vulkan::Buffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
	{
	}

	template<class TUniformBufferData>
	inline UniformBuffer<TUniformBufferData>::UniformBuffer()
		: Core::UniformBuffer<TUniformBufferData>(Core::Buffer::Settings {sizeof(UniformBuffer<TUniformBufferData>::TData), std::string {}}),
		Vulkan::Buffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
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
		ctx->entity = std::dynamic_pointer_cast<Vulkan::Buffer>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	template <class TUniformBufferData>
	std::shared_ptr<Core::BaseTask> UniformBuffer<TUniformBufferData>::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		const auto context = std::make_shared<CopyBufferTaskContext>();
		context->src = std::dynamic_pointer_cast<Vulkan::Buffer>(std::const_pointer_cast<Core::Buffer>(this->GetUnderlyingBuffer()));
		context->dst = std::dynamic_pointer_cast<Vulkan::Buffer>(dst->GetUnderlyingBuffer());
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
			std::memcpy(static_cast<char*>(entity->_deviceMemoryHeapHandle.GetMemoryBlock()->GetHost()) + entity->_deviceMemoryHeapHandle.GetOffset(), std::addressof(tc->data), sizeof(tc->data));
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