#pragma once
#include <Core/Base.hpp>

namespace MMPEngine::Core
{
	class Math
	{
	protected:
		Math();
	public:
		Math(const Math&) = delete;
		Math(Math&&) noexcept = delete;
		Math& operator=(const Math&) = delete;
		Math& operator=(Math&&) noexcept = delete;
		virtual ~Math();

		static constexpr auto kPi = 3.1415926535f;

		static constexpr Matrix4x4 kMatrix4x4Identity = {
			{
		{1.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
			}
		};

		inline static std::float_t ConvertDegreesToRadians(std::float_t degrees);

		virtual std::float_t Dot(const Vector3Float& v1, const Vector3Float& v2) const;
		virtual void Cross(Vector3Float& res, const Vector3Float& v1, const Vector3Float& v2) const;
		virtual void Normalize(Vector3Float& v) const;
		virtual std::float_t SquaredMagnitude(const Vector3Float& v) const;
		virtual std::float_t Magnitude(const Vector3Float& v) const;

		virtual void TRS(Matrix4x4& matrix, const Transform& transform) const;

	private:
		static constexpr auto _deg2Rad = kPi / 180.0f;
	};

	class DefaultMath final : public Math
	{
	};
}