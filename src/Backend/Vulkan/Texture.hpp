#pragma once
#include <Core/Texture.hpp>
#include <Backend/Vulkan/Entity.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class ITargetTexture
	{
	public:
		ITargetTexture();
		ITargetTexture(const ITargetTexture&) = delete;
		ITargetTexture(ITargetTexture&&) noexcept = delete;
		ITargetTexture& operator=(const ITargetTexture&) = delete;
		ITargetTexture& operator=(ITargetTexture&&) noexcept = delete;
		virtual ~ITargetTexture();
	};

	class IDepthStencilTexture : public ITargetTexture
	{
	};

	class IColorTargetTexture : public ITargetTexture
	{
	};

	class BaseTexture : public ResourceEntity
	{
	public:
		virtual std::shared_ptr<Core::BaseTask> CreateMemoryBarrierTask(
			VkAccessFlags srcAccess, 
			VkAccessFlags dstAccess,
			VkImageLayout newLayout,
			const VkImageSubresourceRange& subResourceRange,
			VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		);

		class MemoryBarrierContext final : public Core::EntityTaskContext<BaseTexture>
		{
		public:
			struct Data final
			{
				VkAccessFlags srcAccess = VK_ACCESS_MEMORY_READ_BIT;
				VkAccessFlags dstAccess = VK_ACCESS_MEMORY_WRITE_BIT;
				VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				VkImageSubresourceRange subresourceRange = {};
			};
			Data data;
		};

		class MemoryBarrierTask final : public Task<MemoryBarrierContext>
		{
		public:
			MemoryBarrierTask(const std::shared_ptr<MemoryBarrierContext>& ctx);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	protected:
		VkImage _nativeImage = VK_NULL_HANDLE;
		VkImageLayout _layout = VK_IMAGE_LAYOUT_UNDEFINED;
	};
}
