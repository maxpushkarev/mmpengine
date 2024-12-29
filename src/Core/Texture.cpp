#include <Core/Texture.hpp>

namespace MMPEngine::Core
{
	BaseTexture::BaseTexture(const std::string& name) : BaseEntity(name)
	{
	}

	TargetTexture::TargetTexture(const std::string& name) : BaseTexture(name)
	{
	}

	ScreenTargetTexture::ScreenTargetTexture(const Settings& settings) : TargetTexture("screen_target_texture"), _settings(settings)
	{
	}

	ColorTargetTexture::ColorTargetTexture(const Settings& settings) : TargetTexture(settings.base.name), _settings(settings)
	{
	}

	DepthStencilTargetTexture::DepthStencilTargetTexture(const Settings& settings) : TargetTexture(settings.base.name), _settings(settings)
	{
	}
}