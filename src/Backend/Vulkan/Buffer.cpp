#include <Backend/Vulkan/Buffer.hpp>
#include <cassert>

namespace MMPEngine::Backend::Vulkan
{
	Buffer::Buffer() = default;
	Buffer::~Buffer()
	{
		if(_nativeBuffer)
		{
			vkDestroyBuffer(_device->GetNativeLogical(), _nativeBuffer, nullptr);
		}
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
		bufferInfo.usage = tc->usage;
		bufferInfo.size = static_cast<VkDeviceSize>(tc->byteSize);
		bufferInfo.pNext = nullptr;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.pQueueFamilyIndices = nullptr;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.flags = 0;

		tc->entity->_device = _specificGlobalContext->device;
		vkCreateBuffer(tc->entity->_device->GetNativeLogical(), &bufferInfo, nullptr, &tc->entity->_nativeBuffer);

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

		if(const auto mappedBuffer = std::dynamic_pointer_cast<MappedBuffer>(tc->entity))
		{
			mappedBuffer->Map(
				tc->entity->_deviceMemoryHeapHandle.GetSize(), 
				tc->entity->_deviceMemoryHeapHandle.GetOffset()
			);
		}
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

	MappedBuffer::MappedBuffer() = default;
	MappedBuffer::~MappedBuffer()
	{
		Unmap();
	}

	void MappedBuffer::Map(std::size_t byteSize, std::size_t offset)
	{
		const auto res = vkMapMemory(
			_device->GetNativeLogical(), 
			_deviceMemoryHeapHandle.GetMemoryBlock()->GetNative(), 
			static_cast<VkDeviceSize>(offset),
			static_cast<VkDeviceSize>(byteSize),
			0, 
			&_mappedBufferPtr
		);
		assert(res == VK_SUCCESS);
	}

	void MappedBuffer::Unmap()
	{
		if(_mappedBufferPtr)
		{
			vkUnmapMemory(_device->GetNativeLogical(), _deviceMemoryHeapHandle.GetMemoryBlock()->GetNative());
		}
	}


	UploadBuffer::UploadBuffer(const Settings& settings) : Core::UploadBuffer(settings)
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
			std::memcpy(static_cast<char*>(entity->_mappedBufferPtr) + tc->byteOffset, tc->src, tc->byteLength);
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
		return nullptr;
	}

	std::shared_ptr<Core::BaseTask> UploadBuffer::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->byteSize = GetSettings().byteLength;
		ctx->entity = std::dynamic_pointer_cast<Vulkan::Buffer>(shared_from_this());
		ctx->usage = static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		return std::make_shared<InitTask>(ctx);
	}

	std::shared_ptr<DeviceMemoryHeap> UploadBuffer::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
	{
		return globalContext->uploadBufferHeap;
	}

	ReadBackBuffer::ReadBackBuffer(const Settings& settings) : Core::ReadBackBuffer(settings)
	{
	}

	std::shared_ptr<Core::ContextualTask<Core::ReadBackBuffer::ReadTaskContext>> ReadBackBuffer::CreateReadTask(void* dst, std::size_t byteLength, std::size_t byteOffset)
	{
		return nullptr;
	}

	std::shared_ptr<Core::BaseTask> ReadBackBuffer::CreateCopyToBufferTask(
		const std::shared_ptr<Core::Buffer>& dst,
		std::size_t byteLength,
		std::size_t srcByteOffset,
		std::size_t dstByteOffset) const
	{
		return nullptr;
	}

	std::shared_ptr<Core::BaseTask> ReadBackBuffer::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->byteSize = GetSettings().byteLength;
		ctx->entity = std::dynamic_pointer_cast<Vulkan::Buffer>(shared_from_this());
		ctx->usage = static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		return std::make_shared<InitTask>(ctx);
	}

	std::shared_ptr<DeviceMemoryHeap> ReadBackBuffer::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
	{
		return globalContext->readBackBufferHeap;
	}


	ResidentBuffer::ResidentBuffer(const Settings& settings) : Core::ResidentBuffer(settings)
	{
	}

	std::shared_ptr<Core::BaseTask> ResidentBuffer::CreateCopyToBufferTask(
		const std::shared_ptr<Core::Buffer>& dst,
		std::size_t byteLength,
		std::size_t srcByteOffset,
		std::size_t dstByteOffset) const
	{
		return nullptr;
	}

	std::shared_ptr<Core::BaseTask> ResidentBuffer::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->byteSize = GetSettings().byteLength;
		ctx->entity = std::dynamic_pointer_cast<Vulkan::Buffer>(shared_from_this());
		ctx->usage = static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		return std::make_shared<InitTask>(ctx);
	}

	std::shared_ptr<DeviceMemoryHeap> ResidentBuffer::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
	{
		return globalContext->residentBufferHeap;
	}

}