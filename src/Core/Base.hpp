#pragma once
#include <cstdint>
#include <cmath>
#include <stdexcept>

namespace MMPEngine::Core
{
	enum class PlatformType : std::uint8_t
	{
		Win,
		Mac
	};

	enum class BackendType : std::uint8_t
	{
		Dx12,
		Vulkan,
		Metal
	};

	struct Vector2Uint
	{
		std::uint32_t x = 0;
		std::uint32_t y = 0;
	};

	struct Vector3Uint : Vector2Uint
	{
		std::uint32_t z = 0;
	};

	struct Vector4Uint : Vector3Uint
	{
		std::uint32_t w = 0;
	};

	struct Vector2Float
	{
		std::float_t x = 0.0f;
		std::float_t y = 0.0f;
	};

	struct Vector3Float : Vector2Float
	{
		std::float_t z = 0.0f;
	};

	struct Vector4Float : Vector3Float
	{
		std::float_t w = 0.0f;
	};

	class UnsupportedException final : public std::runtime_error
	{
	public:
		UnsupportedException(const std::string& err);
	};
}
