#pragma once
#include <Core/Base.hpp>

namespace MMPEngine::Core
{
	class Node;

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

		virtual void Scale(Core::Matrix4x4& res, const Core::Vector3Float& scale) const;
		virtual void Translation(Core::Matrix4x4& res, const Core::Vector3Float& translation) const;
		virtual void Rotation(Core::Matrix4x4& res, const Core::Quaternion& rotation) const;
		virtual void TRS(Matrix4x4& matrix, const Transform& transform) const;
		virtual void Multiply(Matrix4x4& res, const Matrix4x4& m1, const Matrix4x4& m2) const;
		virtual void Multiply(Vector4Float& res, const Matrix4x4& m, const Vector4Float& v) const;
		virtual void MultiplyMatrixAndPoint(Core::Vector3Float& res, const Core::Matrix4x4& m, const Core::Vector3Float& p) const;
		virtual void MultiplyMatrixAndVector(Core::Vector3Float& res, const Core::Matrix4x4& m, const Core::Vector3Float& v) const;
		virtual void Transpose(Matrix4x4& res, const Matrix4x4& m) const;
		virtual std::float_t Determinant(const Matrix4x4& m) const;
		virtual void Inverse(Matrix4x4& res, const Matrix4x4& m) const;

		virtual void Normalize(Quaternion& q) const;
		virtual void Inverse(Quaternion& res, const Quaternion& q) const;
		virtual void Multiply(Quaternion& res, const Quaternion& q1, const Quaternion& q2) const;
		virtual std::float_t Dot(const Quaternion& q1, const Quaternion& q2) const;
		virtual void RotationAroundAxis(Quaternion& res, const Vector3Float& v, std::float_t rad) const;

		virtual void FetchLocalToWorldSpaceMatrix(Matrix4x4& res, const std::shared_ptr<Node>& node) const;

	private:
		static constexpr auto _deg2Rad = kPi / 180.0f;
		static constexpr auto _minValidationFloat = 0.000001f;

		static void ConjugateInPlace(Quaternion& q);

		template<typename TMatrix>
		std::float_t DeterminantInternal(const TMatrix& m) const;
	};

	class DefaultMath final : public Math
	{
	};

	template<>
	std::float_t Math::DeterminantInternal<Matrix4x4>(const Matrix4x4& m) const;

	template<>
	std::float_t Math::DeterminantInternal<Matrix3x3>(const Matrix3x3& m) const;

	template<>
	std::float_t Math::DeterminantInternal<Matrix2x2>(const Matrix2x2& m) const;

	template<typename TMatrix>
	inline std::float_t Math::DeterminantInternal(const TMatrix& m) const
	{
		return 0.0f;
	}
}