#pragma once
#include <cstdint>
#include <cmath>
#include <memory>
#include <stdexcept>

namespace MMPEngine::Core
{
	class BaseTask;

	class IInitializationTaskSource
	{
	public:
		IInitializationTaskSource();
		virtual ~IInitializationTaskSource();

		IInitializationTaskSource(const IInitializationTaskSource&) = delete;
		IInitializationTaskSource(IInitializationTaskSource&&) noexcept = delete;
		IInitializationTaskSource& operator=(const IInitializationTaskSource&) = delete;
		IInitializationTaskSource& operator=(IInitializationTaskSource&&) noexcept = delete;

		virtual std::shared_ptr<BaseTask> CreateInitializationTask() = 0;
	};

	class INamed
	{
	public:
		INamed();
		virtual ~INamed();

		INamed(const INamed&) = delete;
		INamed(INamed&&) noexcept = delete;
		INamed& operator=(const INamed&) = delete;
		INamed& operator=(INamed&&) noexcept = delete;

		virtual std::string_view GetName() const = 0;
	};

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

	struct Vector2Uint final
	{
		std::uint32_t x;
		std::uint32_t y;
	};

	struct Vector3Uint final
	{
		std::uint32_t x;
		std::uint32_t y;
		std::uint32_t z;
	};

	struct Vector4Uint final
	{
		std::uint32_t x;
		std::uint32_t y;
		std::uint32_t z;
		std::uint32_t w;
	};

	struct Vector2Float final
	{
		std::float_t x;
		std::float_t y;
	};

	struct Vector3Float final
	{
		std::float_t x;
		std::float_t y;
		std::float_t z;
	};

	struct Vector4Float final
	{
		std::float_t x;
		std::float_t y;
		std::float_t z;
		std::float_t w;
	};

	class UnsupportedException final : public std::runtime_error
	{
	public:
		UnsupportedException(const std::string& err);
	};
}
