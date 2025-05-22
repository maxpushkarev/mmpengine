#pragma once
#include <Core/Texture.hpp>

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
}