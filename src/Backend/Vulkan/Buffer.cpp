#include <Backend/Vulkan/Buffer.hpp>
#include <cassert>

namespace MMPEngine::Backend::Vulkan
{
	Buffer::Buffer(VkBufferUsageFlags usage) : _usage(usage), _info {}
	{
	}

	Buffer::~Buffer()
	{
		if(_nativeBuffer)
		{
			vkDestroyBuffer(_device->GetNativeLogical(), _nativeBuffer, nullptr);
		}
	}

	const VkDescriptorBufferInfo& Buffer::GetDescriptorBufferInfo() const
	{
		return _info;
	}

	Buffer::MemoryBarrierTask::MemoryBarrierTask(const std::shared_ptr<MemoryBarrierContext>& ctx) : Task<MMPEngine::Backend::Vulkan::Buffer::MemoryBarrierContext>(ctx)
	{
	}

	void Buffer::MemoryBarrierTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto tc = GetTaskContext();
		VkBufferMemoryBarrier b {};

		b.pNext = nullptr;
		b.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

		b.buffer = tc->entity->_nativeBuffer;
		b.offset = 0;
		b.size = VK_WHOLE_SIZE;

		b.srcAccessMask = tc->srcAccess;
		b.dstAccessMask = tc->dstAccess;

		b.srcQueueFamilyIndex = _specificStreamContext->GetQueue()->GetFamilyIndex();
		b.dstQueueFamilyIndex = _specificStreamContext->GetQueue()->GetFamilyIndex();

		vkCmdPipelineBarrier(
			_specificStreamContext->PopulateCommandsInBuffer()->GetNative(),
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0, nullptr,
			1, &b,
			0, nullptr
		);
	}

	std::shared_ptr<Core::BaseTask> Buffer::CreateMemoryBarrierTask(VkAccessFlags srcAccess, VkAccessFlags dstAccess)
	{
		const auto ctx = std::make_shared<MemoryBarrierContext>();
		ctx->entity = std::dynamic_pointer_cast<Vulkan::Buffer>(shared_from_this());
		ctx->srcAccess = srcAccess;
		ctx->dstAccess = dstAccess;
		return std::make_shared<MemoryBarrierTask>(ctx);
	}

	Buffer::InitTask::Create::Create(const std::shared_ptr<InitTaskContext>& context) : Task<MMPEngine::Backend::Vulkan::Buffer::InitTaskContext>(context)
	{
	}

	void Buffer::InitTask::Create::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		const auto tc = GetTaskContext();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = tc->entity->_usage;
		bufferInfo.size = static_cast<VkDeviceSize>(tc->byteSize);
		bufferInfo.pNext = nullptr;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.pQueueFamilyIndices = nullptr;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.flags = 0;

		tc->entity->_device = _specificGlobalContext->device;
		vkCreateBuffer(tc->entity->_device->GetNativeLogical(), &bufferInfo, nullptr, &tc->entity->_nativeBuffer);

		tc->entity->_info = {
			tc->entity->_nativeBuffer,
			0,
			bufferInfo.size
		};

		const auto memHeap = tc->entity->GetMemoryHeap(_specificGlobalContext);
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(tc->entity->_device->GetNativeLogical(), tc->entity->_nativeBuffer, &memRequirements);

		tc->entity->_deviceMemoryHeapHandle = memHeap->Allocate(Core::Heap::Request {
			static_cast<std::size_t>(memRequirements.size),
				static_cast<std::size_t>(memRequirements.alignment)
		});
	}

	Buffer::InitTask::Bind::Bind(const std::shared_ptr<InitTaskContext>& context) : Task<MMPEngine::Backend::Vulkan::Buffer::InitTaskContext>(context)
	{
	}

	void Buffer::InitTask::Bind::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto tc = GetTaskContext();

		const auto res = vkBindBufferMemory(
			tc->entity->_device->GetNativeLogical(), 
			tc->entity->_nativeBuffer, 
			tc->entity->_deviceMemoryHeapHandle.GetMemoryBlock()->GetNative(), 
			static_cast<VkDeviceSize>(tc->entity->_deviceMemoryHeapHandle.GetOffset())
		);

		assert(res == VK_SUCCESS);
	}


	Buffer::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& context) : Task<MMPEngine::Backend::Vulkan::Buffer::InitTaskContext>(context)
	{
	}

	void Buffer::InitTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
		stream->Schedule(std::make_shared<Create>(GetTaskContext()));
		stream->Schedule(GetTaskContext()->entity->GetMemoryHeap(_specificGlobalContext)->CreateTaskToInitializeBlocks());
		stream->Schedule(std::make_shared<Bind>(GetTaskContext()));
	}

	Buffer::CopyBufferTask::CopyBufferTask(const std::shared_ptr<CopyBufferTaskContext>& context) : Task<
		MMPEngine::Backend::Vulkan::Buffer::CopyBufferTaskContext>(context)
	{
		const auto tc = GetTaskContext();
		const auto srcBuffer = tc->src;
		const auto dstBuffer = tc->dst;

		assert(srcBuffer);
		assert(dstBuffer);

		_commandTask = std::make_shared<Impl>(context);
		_srcBufferBarrierTask = srcBuffer->CreateMemoryBarrierTask(VkAccessFlagBits::VK_ACCESS_MEMORY_WRITE_BIT, VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT);
		_dstBufferBarrierTask = dstBuffer->CreateMemoryBarrierTask(VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT, VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT);
	}

	void Buffer::CopyBufferTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		stream->Schedule(_srcBufferBarrierTask);
		stream->Schedule(_dstBufferBarrierTask);
		stream->Schedule(_commandTask);
	}

	Buffer::CopyBufferTask::Impl::Impl(const std::shared_ptr<CopyBufferTaskContext>& context) : Task<MMPEngine::Backend::Vulkan::Buffer::CopyBufferTaskContext>(context)
	{
	}

	void Buffer::CopyBufferTask::Impl::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto tc = GetTaskContext();
		const auto srcBuffer = tc->src;
		const auto dstBuffer = tc->dst;

		VkBufferCopy region {};
		region.size = static_cast<VkDeviceSize>(tc->byteLength);
		region.srcOffset = static_cast<VkDeviceSize>(tc->srcByteOffset);
		region.dstOffset = static_cast<VkDeviceSize>(tc->dstByteOffset);

		vkCmdCopyBuffer(
			_specificStreamContext->PopulateCommandsInBuffer()->GetNative(),
			srcBuffer->_nativeBuffer,
			dstBuffer->_nativeBuffer,
			1,
			&region
		);
	}

	UploadBuffer::UploadBuffer(const Settings& settings) : Core::UploadBuffer(settings), Vulkan::Buffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
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
			std::memcpy(static_cast<char*>(entity->_deviceMemoryHeapHandle.GetMemoryBlock()->GetHost()) + entity->_deviceMemoryHeapHandle.GetOffset() + tc->byteOffset, tc->src, tc->byteLength);
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
		context->src = std::dynamic_pointer_cast<Vulkan::Buffer>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
		context->dst = std::dynamic_pointer_cast<Vulkan::Buffer>(dst->GetUnderlyingBuffer());
		context->srcByteOffset = srcByteOffset;
		context->dstByteOffset = dstByteOffset;
		context->byteLength = byteLength;

		return std::make_shared<CopyBufferTask>(context);
	}

	std::shared_ptr<Core::BaseTask> UploadBuffer::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->byteSize = GetSettings().byteLength;
		ctx->entity = std::dynamic_pointer_cast<Vulkan::Buffer>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	std::shared_ptr<DeviceMemoryHeap> UploadBuffer::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
	{
		return globalContext->uploadBufferHeap;
	}

	ReadBackBuffer::ReadBackBuffer(const Settings& settings) : Core::ReadBackBuffer(settings), Vulkan::Buffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
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
			std::memcpy(tc->dst, static_cast<char*>(entity->_deviceMemoryHeapHandle.GetMemoryBlock()->GetHost()) + entity->_deviceMemoryHeapHandle.GetOffset() + tc->byteOffset, tc->byteLength);
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
		context->src = std::dynamic_pointer_cast<Vulkan::Buffer>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
		context->dst = std::dynamic_pointer_cast<Vulkan::Buffer>(dst->GetUnderlyingBuffer());
		context->srcByteOffset = srcByteOffset;
		context->dstByteOffset = dstByteOffset;
		context->byteLength = byteLength;

		return std::make_shared<CopyBufferTask>(context);
	}

	std::shared_ptr<Core::BaseTask> ReadBackBuffer::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->byteSize = GetSettings().byteLength;
		ctx->entity = std::dynamic_pointer_cast<Vulkan::Buffer>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	std::shared_ptr<DeviceMemoryHeap> ReadBackBuffer::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
	{
		return globalContext->readBackBufferHeap;
	}


	ResidentBuffer::ResidentBuffer(const Settings& settings) : Core::ResidentBuffer(settings), Vulkan::Buffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
	{
	}

	std::shared_ptr<Core::BaseTask> ResidentBuffer::CreateCopyToBufferTask(
		const std::shared_ptr<Core::Buffer>& dst,
		std::size_t byteLength,
		std::size_t srcByteOffset,
		std::size_t dstByteOffset) const
	{
		const auto context = std::make_shared<CopyBufferTaskContext>();
		context->src = std::dynamic_pointer_cast<Vulkan::Buffer>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
		context->dst = std::dynamic_pointer_cast<Vulkan::Buffer>(dst->GetUnderlyingBuffer());
		context->srcByteOffset = srcByteOffset;
		context->dstByteOffset = dstByteOffset;
		context->byteLength = byteLength;

		return std::make_shared<CopyBufferTask>(context);
	}

	std::shared_ptr<Core::BaseTask> ResidentBuffer::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->byteSize = GetSettings().byteLength;
		ctx->entity = std::dynamic_pointer_cast<Vulkan::Buffer>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	std::shared_ptr<DeviceMemoryHeap> ResidentBuffer::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
	{
		return globalContext->residentBufferHeap;
	}

	InputAssemblerBuffer::InputAssemblerBuffer(const Core::InputAssemblerBuffer::Settings& settings, const std::shared_ptr<UploadBuffer>& upload, const std::shared_ptr<Core::Buffer>& storage)
		: _upload(upload), _storage(storage), _ia(settings.ia)
	{
	}

	InputAssemblerBuffer::~InputAssemblerBuffer() = default;

	InputAssemblerBuffer::InitTask::InitTask(const std::shared_ptr<TaskContext>& context) : Task(context)
	{
	}

	void InputAssemblerBuffer::InitTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		if (const auto tc = GetTaskContext(); const auto entity = tc->entity)
		{
			stream->Schedule(entity->_upload->CreateInitializationTask());
			stream->Schedule(entity->_storage->CreateInitializationTask());
			stream->Schedule(Core::StreamBarrierTask::kInstance);
			stream->Schedule(entity->_upload->CreateWriteTask(entity->_ia.rawData, entity->_upload->GetSettings().byteLength, 0));
			stream->Schedule(entity->_upload->CopyToBuffer(entity->_storage));
		}
	}

	void InputAssemblerBuffer::InitTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);

		if (const auto tc = GetTaskContext(); const auto entity = tc->entity)
		{
			entity->_upload.reset();
		}
	}

	IndexBuffer::IndexBuffer(const Core::InputAssemblerBuffer::Settings& settings) : Core::IndexBuffer(settings), Vulkan::InputAssemblerBuffer(settings, std::make_shared<UploadBuffer>(settings.base), std::make_shared<Internal>(settings.base))
	{
	}

	std::shared_ptr<Core::BaseTask> IndexBuffer::CreateInitializationTask()
	{
		const auto context = std::make_shared<TaskContext>();
		context->entity = std::dynamic_pointer_cast<Vulkan::InputAssemblerBuffer>(shared_from_this());
		return std::make_shared<InitTask>(context);
	}

	std::shared_ptr<Core::Buffer> IndexBuffer::GetUnderlyingBuffer()
	{
		return _storage;
	}

	std::shared_ptr<Core::BaseTask> IndexBuffer::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		return _storage->CreateCopyToBufferTask(dst, byteLength, srcByteOffset, dstByteOffset);
	}

	IndexBuffer::Internal::Internal(const Settings& settings) : Core::ResidentBuffer(settings), Vulkan::Buffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
	{
	}

	std::shared_ptr<Core::BaseTask> IndexBuffer::Internal::CreateCopyToBufferTask(
		const std::shared_ptr<Core::Buffer>& dst,
		std::size_t byteLength,
		std::size_t srcByteOffset,
		std::size_t dstByteOffset) const
	{
		const auto context = std::make_shared<CopyBufferTaskContext>();
		context->src = std::dynamic_pointer_cast<Vulkan::Buffer>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
		context->dst = std::dynamic_pointer_cast<Vulkan::Buffer>(dst->GetUnderlyingBuffer());
		context->srcByteOffset = srcByteOffset;
		context->dstByteOffset = dstByteOffset;
		context->byteLength = byteLength;

		return std::make_shared<CopyBufferTask>(context);
	}

	std::shared_ptr<Core::BaseTask> IndexBuffer::Internal::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->byteSize = GetSettings().byteLength;
		ctx->entity = std::dynamic_pointer_cast<Vulkan::Buffer>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	std::shared_ptr<DeviceMemoryHeap> IndexBuffer::Internal::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
	{
		return globalContext->residentBufferHeap;
	}


	VertexBuffer::VertexBuffer(const Core::InputAssemblerBuffer::Settings& settings) : Core::VertexBuffer(settings), Vulkan::InputAssemblerBuffer(settings, std::make_shared<UploadBuffer>(settings.base), std::make_shared<Internal>(settings.base))
	{
	}

	std::shared_ptr<Core::BaseTask> VertexBuffer::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		return _storage->CreateCopyToBufferTask(dst, byteLength, srcByteOffset, dstByteOffset);
	}

	std::shared_ptr<Core::BaseTask> VertexBuffer::CreateInitializationTask()
	{
		const auto context = std::make_shared<TaskContext>();
		context->entity = std::dynamic_pointer_cast<Vulkan::InputAssemblerBuffer>(shared_from_this());
		return std::make_shared<InitTask>(context);
	}

	std::shared_ptr<Core::Buffer> VertexBuffer::GetUnderlyingBuffer()
	{
		return _storage;
	}

	VertexBuffer::Internal::Internal(const Settings& settings) : Core::ResidentBuffer(settings), Vulkan::Buffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
	{
	}

	std::shared_ptr<Core::BaseTask> VertexBuffer::Internal::CreateCopyToBufferTask(
		const std::shared_ptr<Core::Buffer>& dst,
		std::size_t byteLength,
		std::size_t srcByteOffset,
		std::size_t dstByteOffset) const
	{
		const auto context = std::make_shared<CopyBufferTaskContext>();
		context->src = std::dynamic_pointer_cast<Vulkan::Buffer>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
		context->dst = std::dynamic_pointer_cast<Vulkan::Buffer>(dst->GetUnderlyingBuffer());
		context->srcByteOffset = srcByteOffset;
		context->dstByteOffset = dstByteOffset;
		context->byteLength = byteLength;

		return std::make_shared<CopyBufferTask>(context);
	}

	std::shared_ptr<Core::BaseTask> VertexBuffer::Internal::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->byteSize = GetSettings().byteLength;
		ctx->entity = std::dynamic_pointer_cast<Vulkan::Buffer>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	std::shared_ptr<DeviceMemoryHeap> VertexBuffer::Internal::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
	{
		return globalContext->residentBufferHeap;
	}


	UnorderedAccessBuffer::UnorderedAccessBuffer(const Settings& settings) : Core::UnorderedAccessBuffer(settings), Vulkan::Buffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
	{
	}

	std::shared_ptr<Core::BaseTask> UnorderedAccessBuffer::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		const auto context = std::make_shared<CopyBufferTaskContext>();
		context->src = std::dynamic_pointer_cast<Vulkan::Buffer>(std::const_pointer_cast<Core::Buffer>(GetUnderlyingBuffer()));
		context->dst = std::dynamic_pointer_cast<Vulkan::Buffer>(dst->GetUnderlyingBuffer());
		context->srcByteOffset = srcByteOffset;
		context->dstByteOffset = dstByteOffset;
		context->byteLength = byteLength;

		return std::make_shared<CopyBufferTask>(context);
	}

	std::shared_ptr<Core::BaseTask> UnorderedAccessBuffer::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->byteSize = GetSettings().byteLength;
		ctx->entity = std::dynamic_pointer_cast<Vulkan::Buffer>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	std::shared_ptr<DeviceMemoryHeap> UnorderedAccessBuffer::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
	{
		return globalContext->residentBufferHeap;
	}


}