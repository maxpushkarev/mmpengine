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
		std::uint32_t x;
		std::uint32_t y;
	};

	struct Vector3Uint : Vector2Uint
	{
		std::uint32_t z;
	};

	struct Vector4Uint : Vector3Uint
	{
		std::uint32_t w;
	};

	struct Vector2Float
	{
		std::float_t x;
		std::float_t y;
	};

	struct Vector3Float : Vector2Float
	{
		std::float_t z;
	};

	struct Vector4Float : Vector3Float
	{
		std::float_t w;
	};

	class UnsupportedException final : public std::runtime_error
	{
	public:
		UnsupportedException(const std::string& err);
	};
}
