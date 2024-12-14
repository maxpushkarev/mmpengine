#pragma once
#include <cstdint>
#include <cmath>
#include <memory>
#include <stdexcept>

namespace MMPEngine::Core
{
	class BaseTask;

	class Constants final
	{
	public:
		static constexpr auto kFloatEps = 0.0001f;	
	};

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

		bool operator==(const Vector2Uint& rhs) const;
		bool operator!=(const Vector2Uint& rhs) const;
	};

	struct Vector3Uint final
	{
		std::uint32_t x;
		std::uint32_t y;
		std::uint32_t z;

		bool operator==(const Vector3Uint& rhs) const;
		bool operator!=(const Vector3Uint& rhs) const;
	};

	struct Vector4Uint final
	{
		std::uint32_t x;
		std::uint32_t y;
		std::uint32_t z;
		std::uint32_t w;

		bool operator==(const Vector4Uint& rhs) const;
		bool operator!=(const Vector4Uint& rhs) const;
	};


	struct Vector2Int final
	{
		std::int32_t x;
		std::int32_t y;

		bool operator==(const Vector2Int& rhs) const;
		bool operator!=(const Vector2Int& rhs) const;
	};

	struct Vector3Int final
	{
		std::int32_t x;
		std::int32_t y;
		std::int32_t z;

		bool operator==(const Vector3Int& rhs) const;
		bool operator!=(const Vector3Int& rhs) const;
	};

	struct Vector4Int final
	{
		std::int32_t x;
		std::int32_t y;
		std::int32_t z;
		std::int32_t w;

		bool operator==(const Vector4Int& rhs) const;
		bool operator!=(const Vector4Int& rhs) const;
	};


	struct Vector2Float final
	{
		std::float_t x;
		std::float_t y;

		bool operator==(const Vector2Float& rhs) const;
		bool operator!=(const Vector2Float& rhs) const;
	};

	struct Vector3Float final
	{
		std::float_t x;
		std::float_t y;
		std::float_t z;

		bool operator==(const Vector3Float& rhs) const;
		bool operator!=(const Vector3Float& rhs) const;
	};

	struct Vector4Float final
	{
		std::float_t x;
		std::float_t y;
		std::float_t z;
		std::float_t w;

		bool operator==(const Vector4Float& rhs) const;
		bool operator!=(const Vector4Float& rhs) const;
	};

	struct Quaternion final
	{
		std::float_t x;
		std::float_t y;
		std::float_t z;
		std::float_t w;

		bool operator==(const Quaternion& rhs) const;
		bool operator!=(const Quaternion& rhs) const;
	};

	struct Matrix4x4 final
	{
		std::float_t m[4][4];

		bool operator==(const Matrix4x4& rhs) const;
		bool operator!=(const Matrix4x4& rhs) const;
	};

	struct Transform final
	{
		Vector3Float position = {0.0f, 0.0f, 0.0f};
		Quaternion rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
		Vector3Float scale = {1.0f, 1.0f, 1.0f};
	};

	class UnsupportedException final : public std::runtime_error
	{
	public:
		UnsupportedException(const std::string& err);
	};
}
