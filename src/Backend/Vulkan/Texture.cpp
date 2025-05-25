#include <Backend/Vulkan/Texture.hpp>

namespace MMPEngine::Backend::Vulkan
{
	ITargetTexture::ITargetTexture() = default;
	ITargetTexture::~ITargetTexture() = default;

	std::shared_ptr<Core::BaseTask> BaseTexture::CreateMemoryBarrierTask(VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkImageLayout newLayout, VkImageSubresourceRange subResourceRange)
	{
		const auto ctx = std::make_shared<MemoryBarrierContext>();
		ctx->entity = std::dynamic_pointer_cast<BaseTexture>(shared_from_this());
		ctx->srcAccess = srcAccess;
		ctx->dstAccess = dstAccess;
		ctx->imageLayout = newLayout;
		ctx->subresourceRange = subResourceRange;

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
		barrier.newLayout = ctx->imageLayout;

		barrier.srcAccessMask = ctx->srcAccess;
		barrier.dstAccessMask = ctx->dstAccess;

		barrier.image = entity->_nativeImage;
		barrier.subresourceRange = ctx->subresourceRange;

		barrier.srcQueueFamilyIndex = ctx->entity->_queueFamilyIndexOwnerShip.value_or(_specificStreamContext->GetQueue()->GetFamilyIndex());
		barrier.dstQueueFamilyIndex = _specificStreamContext->GetQueue()->GetFamilyIndex();

		vkCmdPipelineBarrier(
			_specificStreamContext->PopulateCommandsInBuffer()->GetNative(),
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		ctx->entity->_queueFamilyIndexOwnerShip = _specificStreamContext->GetQueue()->GetFamilyIndex();
		ctx->entity->_layout = ctx->imageLayout;
	}


}