#include <Backend/Vulkan/Texture.hpp>

namespace MMPEngine::Backend::Vulkan
{
	ITargetTexture::ITargetTexture() = default;
	ITargetTexture::~ITargetTexture() = default;

	std::shared_ptr<Core::BaseTask> BaseTexture::CreateMemoryBarrierTask(
		VkAccessFlags srcAccess, 
		VkAccessFlags dstAccess, 
		VkImageLayout newLayout, 
		const VkImageSubresourceRange& subResourceRange,
		VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
	{
		const auto ctx = std::make_shared<MemoryBarrierContext>();
		ctx->entity = std::dynamic_pointer_cast<BaseTexture>(shared_from_this());
		ctx->data.srcAccess = srcAccess;
		ctx->data.dstAccess = dstAccess;
		ctx->data.srcStage = srcStage;
		ctx->data.dstStage = dstStage;
		ctx->data.imageLayout = newLayout;
		ctx->data.subresourceRange = subResourceRange;

		return std::make_shared<MemoryBarrierTask>(ctx);
	}

	BaseTexture::MemoryBarrierTask::MemoryBarrierTask(const std::shared_ptr<MemoryBarrierContext>& ctx) : Task<MMPEngine::Backend::Vulkan::BaseTexture::MemoryBarrierContext>(ctx)
	{
	}

	void BaseTexture::MemoryBarrierTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto ctx = GetTaskContext();
		const auto entity = ctx->entity;

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;

		barrier.oldLayout = entity->_layout;
		barrier.newLayout = ctx->data.imageLayout;

		barrier.srcAccessMask = ctx->data.srcAccess;
		barrier.dstAccessMask = ctx->data.dstAccess;

		barrier.image = entity->_nativeImage;
		barrier.subresourceRange = ctx->data.subresourceRange;

		barrier.srcQueueFamilyIndex = ctx->entity->_queueFamilyIndexOwnerShip.value_or(_specificStreamContext->GetQueue()->GetFamilyIndex());
		barrier.dstQueueFamilyIndex = _specificStreamContext->GetQueue()->GetFamilyIndex();

		vkCmdPipelineBarrier(
			_specificStreamContext->PopulateCommandsInBuffer()->GetNative(),
			ctx->data.srcStage,
			ctx->data.dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		ctx->entity->_queueFamilyIndexOwnerShip = _specificStreamContext->GetQueue()->GetFamilyIndex();
		ctx->entity->_layout = ctx->data.imageLayout;
	}


	DepthStencilTargetTexture::DepthStencilTargetTexture(const Settings& settings) : Core::DepthStencilTargetTexture(settings)
	{
	}

	std::shared_ptr<DeviceMemoryHeap> DepthStencilTargetTexture::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
	{
		return nullptr;
	}

	std::shared_ptr<Core::BaseTask> DepthStencilTargetTexture::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<DepthStencilTargetTexture>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	DepthStencilTargetTexture::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task(ctx)
	{
	}

	void DepthStencilTargetTexture::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		//const auto dsTex = GetTaskContext()->entity;
	}



}