#include <Core/Texture.hpp>

namespace MMPEngine::Core
{
	BaseTexture::BaseTexture(const std::string& name) : BaseEntity(name)
	{
	}

	std::shared_ptr<BaseTexture> BaseTexture::GetUnderlyingTexture() const
	{
		const auto underlyingTexture = const_cast<BaseTexture*>(this)->GetUnderlyingTexture();
		return underlyingTexture;
	}

	std::shared_ptr<BaseTexture> BaseTexture::GetUnderlyingTexture()
	{
		return std::dynamic_pointer_cast<BaseTexture>(shared_from_this());
	}


	TargetTexture::TargetTexture(const std::string& name) : BaseTexture(name)
	{
	}

    const ColorTargetTexture::Settings& ColorTargetTexture::GetSettings() const
    {
        return _settings;
    }

    ColorTargetTexture::ColorTargetTexture(const Settings& settings) : TargetTexture(settings.base.name), _settings(settings)
	{
	}

	const DepthStencilTargetTexture::Settings& DepthStencilTargetTexture::GetSettings() const
	{
		return _settings;
	}

	DepthStencilTargetTexture::DepthStencilTargetTexture(const Settings& settings) : TargetTexture(settings.base.name), _settings(settings)
	{
	}
}