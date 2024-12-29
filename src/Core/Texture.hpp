#pragma once
#include <Core/Entity.hpp>

namespace MMPEngine::Core
{
	class BaseTexture : public BaseEntity
	{
	protected:
		BaseTexture(const std::string& name);
	};

	class TargetTexture : public BaseTexture
	{
	public:
		struct Settings final
		{
			enum class Antialiasing : std::uint8_t
			{
				MSAA_0,
				MSAA_2,
				MSAA_4,
				MSAA_8
			};

			Antialiasing antialiasing = Antialiasing::MSAA_0;
			Vector2Uint size{ 0,0 };
			std::string name {};
		};
	protected:
		TargetTexture(const std::string& name);
	};

	class ColorTargetTexture : public TargetTexture
	{
	public:
		struct Settings final
		{
			enum class Format : std::uint8_t
			{
				R8G8B8A8_Float_01,
				R32G32B32A32_Float_Unbound,
				R32G32B32_Float_Unbound,
				R32G32_Float_Unbound,
				R32_Float_Unbound
			};

			Format format = Format::R8G8B8A8_Float_01;
			TargetTexture::Settings base;
		};
	protected:
		ColorTargetTexture(const Settings& settings);
	protected:
		Settings _settings;
	};

	class DepthStencilTargetTexture : public TargetTexture
	{
	public:
		struct Settings final
		{
			enum class Format : std::uint8_t
			{
				Depth24_Stencil8,
				Depth16,
				Depth32
			};

			Format format = Format::Depth24_Stencil8;
			TargetTexture::Settings base;
		};
	protected:
		DepthStencilTargetTexture(const Settings& settings);
	protected:
		Settings _settings;
	};
}