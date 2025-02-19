#include <Backend/Vulkan/Buffer.hpp>

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


	Buffer::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& context) : Task<MMPEngine::Backend::Vulkan::Buffer::InitTaskContext>(context)
	{
	}

	void Buffer::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

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
	}


	UploadBuffer::UploadBuffer(const Settings& settings) : Core::UploadBuffer(settings)
	{
	}

	std::shared_ptr<Core::ContextualTask<Core::UploadBuffer::WriteTaskContext>> UploadBuffer::CreateWriteTask(const void* src, std::size_t byteLength, std::size_t byteOffset)
	{
		return nullptr;
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
}