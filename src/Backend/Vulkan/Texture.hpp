#pragma once
#include <Core/Texture.hpp>
#include <vulkan/vulkan.h>

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

		/*virtual std::shared_ptr<Core::BaseTask> CreateBarrierTask(
			VkAccessFlags srcAccess, 
			VkAccessFlags dstAccess,
			VkImageLayout srcLayout,
			VkImageLayout dstLayout
		);*/
	};

	class IDepthStencilTexture : public ITargetTexture
	{
	};

	class IColorTargetTexture : public ITargetTexture
	{
	};
}
