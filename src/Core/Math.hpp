#pragma once
#include <vector>
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

		static constexpr Quaternion kQuaternionIdentity = {
			0.0f, 0.0f, 0.0f, 1.0f
		};

		static std::float_t ConvertDegreesToRadians(std::float_t degrees);

		virtual std::float_t Dot(const Vector3Float& v1, const Vector3Float& v2) const;
		virtual void Cross(Vector3Float& res, const Vector3Float& v1, const Vector3Float& v2) const;
		virtual void Normalize(Vector3Float& v) const;
		virtual std::float_t SquaredMagnitude(const Vector3Float& v) const;
		virtual std::float_t Magnitude(const Vector3Float& v) const;
		virtual void Project(Vector3Float& res, const Vector3Float& v, const Vector3Float& p) const;

		virtual void TRS(Matrix4x4& matrix, const Transform& transform) const;
		virtual void Multiply(Matrix4x4& res, const Matrix4x4& m1, const Matrix4x4& m2) const;
		virtual void Multiply(Vector4Float& res, const Matrix4x4& m, const Vector4Float& v) const;
		virtual void MultiplyMatrixAndPoint(Core::Vector3Float& res, const Core::Matrix4x4& m, const Core::Vector3Float& p) const;
		virtual void MultiplyMatrixAndVector(Core::Vector3Float& res, const Core::Matrix4x4& m, const Core::Vector3Float& v) const;
		virtual void Transpose(Matrix4x4& res, const Matrix4x4& m) const;
		virtual std::float_t Determinant(const Matrix4x4& m) const;
		virtual void Inverse(Matrix4x4& res, const Matrix4x4& m) const;

	private:
		static constexpr auto _deg2Rad = kPi / 180.0f;
		static constexpr auto _minDetAbs = 0.000001f;

		template<typename TMatrix>
		static std::float_t DeterminantInternal(const TMatrix& m);


		template<>
		static std::float_t DeterminantInternal<Matrix4x4>(const Matrix4x4& m);

		template<>
		static std::float_t DeterminantInternal<Matrix3x3>(const Matrix3x3& m);

		template<>
		static std::float_t DeterminantInternal<Matrix2x2>(const Matrix2x2& m);
	};

	class DefaultMath final : public Math
	{
	};


	template<typename TMatrix>
	inline std::float_t Math::DeterminantInternal(const TMatrix& m)
	{
		return 0.0f;
	}
}