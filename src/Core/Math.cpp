#include <Core/Math.hpp>
#include <Core/Node.hpp>
#include <cassert>

namespace MMPEngine::Core
{
	Math::Math() = default;
	Math::~Math() = default;

	std::float_t Math::ConvertDegreesToRadians(std::float_t degrees)
	{
		return degrees * _deg2Rad;
	}

	void Math::Cross(Vector3Float& res, const Vector3Float& v1, const Vector3Float& v2) const
	{
		res.x = v1.y * v2.z - v1.z * v2.y;
		res.y = v1.z * v2.x - v1.x * v2.z;
		res.z = v1.x * v2.y - v1.y * v2.x;
	}

	std::float_t Math::Dot(const Vector3Float& v1, const Vector3Float& v2) const
	{
		return v1.x * v2.x + 
				v1.y * v2.y + 
				v1.z * v2.z;
	}

	std::float_t Math::Magnitude(const Vector3Float& v) const
	{
		return std::sqrtf(SquaredMagnitude(v));
	}

	std::float_t Math::SquaredMagnitude(const Vector3Float& v) const
	{
		return v.x * v.x + 
				v.y * v.y + 
				v.z * v.z;
	}

	void Math::Normalize(Vector3Float& v) const
	{
		const auto m = Magnitude(v);

		if(m >= Constants::kFloatEps)
		{
			const auto f = 1.0f / m;
			v.x *= f;
			v.y *= f;
			v.z *= f;
		}
		else
		{
			v = {0.0f, 0.0f, 0.0f};
		}
	}

	void Math::Project(Vector3Float& res, const Vector3Float& v, const Vector3Float& p) const
	{
		const auto dot = Dot(v, p);
		const auto sqrMagnitude = SquaredMagnitude(p);

		if(sqrMagnitude < Constants::kFloatEps)
		{
			res = {0.0f, 0.0f, 0.0f};
			return;
		}

		const auto multiplier = dot / sqrMagnitude;
		res = p;
		res.x *= multiplier;
		res.y *= multiplier;
		res.z *= multiplier;
	}


	void Math::TRS(Matrix4x4& matrix, const Transform& transform) const
	{
		Matrix4x4 translation {};
		Matrix4x4 rotation {};
		Matrix4x4 scale {};

		Translation(translation, transform.position);
		Rotation(rotation, transform.rotation);
		Scale(scale, transform.scale);

		Matrix4x4 tmp {};
		Multiply(tmp, translation, rotation);
		Multiply(matrix, tmp, scale);
	}

	void Math::DecomposeTRS(Transform& transform, const Matrix4x4& matrix) const
	{
		Matrix4x4 tmp = matrix;

		transform.position.x = tmp.m[0][3];
		transform.position.y = tmp.m[1][3];
		transform.position.z = tmp.m[2][3];

		tmp.m[0][3] = 0.0f;
		tmp.m[1][3] = 0.0f;
		tmp.m[2][3] = 0.0f;
		tmp.m[3][3] = 1.0f;

		transform.scale.x = Magnitude(Vector3Float{ tmp.m[0][0], tmp.m[1][0], tmp.m[2][0] });
		transform.scale.y = Magnitude(Vector3Float{ tmp.m[0][1], tmp.m[1][1], tmp.m[2][1] });
		transform.scale.z = Magnitude(Vector3Float{ tmp.m[0][2], tmp.m[1][2], tmp.m[2][2] });

		assert(std::abs(transform.scale.x) >= _minValidationFloat);
		assert(std::abs(transform.scale.y) >= _minValidationFloat);
		assert(std::abs(transform.scale.z) >= _minValidationFloat);

		const Vector3Float invScale = {1.0f / transform.scale.x, 1.0f / transform.scale.y, 1.0f / transform.scale.z};

		tmp.m[0][0] *= invScale.x;
		tmp.m[1][0] *= invScale.x;
		tmp.m[2][0] *= invScale.x;

		tmp.m[0][1] *= invScale.y;
		tmp.m[1][1] *= invScale.y;
		tmp.m[2][1] *= invScale.y;

		tmp.m[0][2] *= invScale.z;
		tmp.m[1][2] *= invScale.z;
		tmp.m[2][2] *= invScale.z;

		// Shepperd's method
		// https://ahrs.readthedocs.io/en/latest/special/Shepperd.html
		const auto r11 = tmp.m[0][0];
		const auto r12 = tmp.m[0][1];
		const auto r13 = tmp.m[0][2];

		const auto r21 = tmp.m[1][0];
		const auto r22 = tmp.m[1][1];
		const auto r23 = tmp.m[1][2];

		const auto r31 = tmp.m[2][0];
		const auto r32 = tmp.m[2][1];
		const auto r33 = tmp.m[2][2];

		const std::float_t u[]
		{
			r11 + r22 + r33,
			r11,
			r22,
			r33
		};

		constexpr auto qMult = 0.5f;
		auto maxIdx = std::size(u);

		for(std::size_t i = 0; i < std::size(u); ++i)
		{
			if(maxIdx == std::size(u) || u[maxIdx] < u[i])
			{
				maxIdx = i;
			}
		}

		assert(maxIdx < std::size(u));

		switch (maxIdx)
		{
		case 0:
			{
				const auto f = 1.0f + r11 + r22 + r33;
				assert(f >= _minValidationFloat);
				const auto sqrtF = std::sqrtf(f);
				assert(sqrtF >= _minValidationFloat);

				transform.rotation.w = qMult * sqrtF;
				transform.rotation.x = qMult * (r32 - r23) / sqrtF;
				transform.rotation.y = qMult * (r13 - r31) / sqrtF;
				transform.rotation.z = qMult * (r21 - r12) / sqrtF;
				break;
			}
		case 1:
			{
				const auto f = 1.0f + r11 - r22 - r33;
				assert(f >= _minValidationFloat);
				const auto sqrtF = std::sqrtf(f);
				assert(sqrtF >= _minValidationFloat);

				transform.rotation.w = qMult * (r32 - r23) / sqrtF;
				transform.rotation.x = qMult * sqrtF;
				transform.rotation.y = qMult * (r12 + r21) / sqrtF;
				transform.rotation.z = qMult * (r31 + r13) / sqrtF;
				break;
			}
		case 2:
			{
				const auto f = 1.0f - r11 + r22 - r33;
				assert(f >= _minValidationFloat);
				const auto sqrtF = std::sqrtf(f);
				assert(sqrtF >= _minValidationFloat);

				transform.rotation.w = qMult * (r13 - r31) / sqrtF;
				transform.rotation.x = qMult * (r12 + r21) / sqrtF;
				transform.rotation.y = qMult * sqrtF;
				transform.rotation.z = qMult * (r23 + r32) / sqrtF;
				break;
			}
		case 3:
			{
				const auto f = 1.0f - r11 - r22 + r33;
				assert(f >= _minValidationFloat);
				const auto sqrtF = std::sqrtf(f);
				assert(sqrtF >= _minValidationFloat);

				transform.rotation.w = qMult * (r21 - r12) / sqrtF;
				transform.rotation.x = qMult * (r31 + r13) / sqrtF;
				transform.rotation.y = qMult * (r32 + r23) /sqrtF;
				transform.rotation.z = qMult * sqrtF;
				break;
			}
		default:
			{
				//impossible
				break;
			}
		}
	}

	void Math::Scale(Core::Matrix4x4& res, const Core::Vector3Float& scale) const
	{
		res = kMatrix4x4Identity;
		res.m[0][0] = scale.x;
		res.m[1][1] = scale.y;
		res.m[2][2] = scale.z;
	}

	void Math::Translation(Core::Matrix4x4& res, const Core::Vector3Float& translation) const
	{
		res = kMatrix4x4Identity;
		res.m[0][3] = translation.x;
		res.m[1][3] = translation.y;
		res.m[2][3] = translation.z;
	}

	void Math::Rotation(Core::Matrix4x4& res, const Core::Quaternion& rotation) const
	{
		Quaternion q = rotation;
		Normalize(q);

		res = kMatrix4x4Identity;

		res.m[0][0] = 1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z;
		res.m[0][1] = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
		res.m[0][2] = 2.0f * q.x * q.z + 2.0f * q.w * q.y;

		res.m[1][0] = 2.0f * q.x * q.y + 2.0f * q.w * q.z;
		res.m[1][1] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
		res.m[1][2] = 2.0f * q.y * q.z - 2.0f * q.w * q.x;

		res.m[2][0] = 2.0f * q.x * q.z - 2.0f * q.w * q.y;
		res.m[2][1] = 2.0f * q.y * q.z + 2.0f * q.w * q.x;
		res.m[2][2] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;
	}

	void Math::Multiply(Matrix4x4& res, const Matrix4x4& m1, const Matrix4x4& m2) const
	{
		res.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0];
		res.m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1];
		res.m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2];
		res.m[0][3] = m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3];

		res.m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0];
		res.m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1];
		res.m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2];
		res.m[1][3] = m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3];

		res.m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0];
		res.m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1];
		res.m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2];
		res.m[2][3] = m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3];

		res.m[3][0] = m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0];
		res.m[3][1] = m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1];
		res.m[3][2] = m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2];
		res.m[3][3] = m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3];
	}

	template<>
	std::float_t Math::DeterminantInternal<Matrix4x4>(const Matrix4x4& m) const
	{
		return
			m.m[0][0] * DeterminantInternal(Matrix3x3 {
				{
					{m.m[1][1],m.m[1][2],m.m[1][3]},
					{m.m[2][1],m.m[2][2],m.m[2][3]},
					{m.m[3][1],m.m[3][2],m.m[3][3]}
				}
			})
			- m.m[0][1] * DeterminantInternal(Matrix3x3{
				{
					{m.m[1][0],m.m[1][2],m.m[1][3]},
					{m.m[2][0],m.m[2][2],m.m[2][3]},
					{m.m[3][0],m.m[3][2],m.m[3][3]}
				}
				})
			+ m.m[0][2] * DeterminantInternal(Matrix3x3{
				{
					{m.m[1][0],m.m[1][1],m.m[1][3]},
					{m.m[2][0],m.m[2][1],m.m[2][3]},
					{m.m[3][0],m.m[3][1],m.m[3][3]}
				}
				})
			- m.m[0][3] * DeterminantInternal(Matrix3x3{
				{
					{m.m[1][0],m.m[1][1],m.m[1][2]},
					{m.m[2][0],m.m[2][1],m.m[2][2]},
					{m.m[3][0],m.m[3][1],m.m[3][2]}
				}
				});
	}

	template<>
	std::float_t Math::DeterminantInternal<Matrix3x3>(const Matrix3x3& m) const
	{
		return
			m.m[0][0] * DeterminantInternal(Matrix2x2 {
				{
					{m.m[1][1],m.m[1][2]},
					{m.m[2][1],m.m[2][2]}
				}
			})
			- m.m[0][1] * DeterminantInternal(Matrix2x2{
				{
					{m.m[1][0],m.m[1][2]},
					{m.m[2][0],m.m[2][2]}
				}
				})
			+ m.m[0][2] * DeterminantInternal(Matrix2x2{
				{
					{m.m[1][0],m.m[1][1]},
					{m.m[2][0],m.m[2][1]}
				}
				});
	}

	template<>
	std::float_t Math::DeterminantInternal<Matrix2x2>(const Matrix2x2& m) const
	{
		return m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];
	}


	std::float_t Math::Determinant(const Matrix4x4& m) const
	{
		return DeterminantInternal(m);
	}

	void Math::Inverse(Matrix4x4& res, const Matrix4x4& m) const
	{
		const auto det = Determinant(m);
		assert(std::abs(det) >= _minValidationFloat);
		const auto invDet = 1.0f / det;

		Matrix4x4 t {};
		Transpose(t, m);

		for(std::size_t i = 0; i < 4; ++i)
		{
			for(std::size_t j = 0; j < 4; ++j)
			{
				const std::float_t sign = ((i + j) % 2 == 0) ? 1.0f : -1.0f;
				Matrix3x3 tmp {};

				std::size_t tmpRow = 0;
				std::size_t tmpCol = 0;

				for(std::size_t tmpI = 0; tmpI < 4; ++tmpI)
				{
					if(tmpI == i)
					{
						continue;
					}
					
					for(std::size_t tmpJ = 0; tmpJ < 4; ++tmpJ)
					{
						if(tmpJ == j)
						{
							continue;
						}

						tmp.m[tmpRow][tmpCol] = t.m[tmpI][tmpJ];
						++tmpCol;
					}

					++tmpRow;
					tmpCol = 0;
				}

				assert(tmpRow == 3);
				assert(tmpCol == 0);

				const auto cofactor = sign * Core::Math::DeterminantInternal(tmp);
				res.m[i][j] = invDet * cofactor;
			}
		}
	}

	void Math::Transpose(Matrix4x4& res, const Matrix4x4& m) const
	{
		for(std::size_t i = 0; i < 4; ++i)
		{
			for(std::size_t j = 0; j < 4; ++j)
			{
				res.m[i][j] = m.m[j][i];
			}
		}
	}

	void Math::MultiplyMatrixAndPoint(Core::Vector3Float& res, const Core::Matrix4x4& m, const Core::Vector3Float& p) const
	{
		res.x = m.m[0][0] * p.x + m.m[0][1] * p.y + m.m[0][2] * p.z + m.m[0][3];
		res.y = m.m[1][0] * p.x + m.m[1][1] * p.y + m.m[1][2] * p.z + m.m[1][3];
		res.z = m.m[2][0] * p.x + m.m[2][1] * p.y + m.m[2][2] * p.z + m.m[2][3];
		auto w = m.m[3][0] * p.x + m.m[3][1] * p.y + m.m[3][2] * p.z + m.m[3][3];

		w = 1.0f / w;
		res.x *= w;
		res.y *= w;
		res.z *= w;
	}

	void Math::MultiplyMatrixAndVector(Core::Vector3Float& res, const Core::Matrix4x4& m, const Core::Vector3Float& v) const
	{
		res.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z;
		res.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z;
		res.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z;
	}

	void Math::Multiply(Vector4Float& res, const Matrix4x4& m, const Vector4Float& v) const
	{
		res.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w;
		res.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w;
		res.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w;
		res.w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w;
	}


	void Math::Normalize(Quaternion& q) const
	{
		const auto mag = std::sqrtf(Dot(q, q));

		if (mag < _minValidationFloat)
		{
			q = kQuaternionIdentity;
		}

		const auto invMag = 1.0f / mag;
		q.x *= invMag;
		q.y *= invMag;
		q.z *= invMag;
		q.w *= invMag;
	}

	std::float_t Math::Dot(const Quaternion& q1, const Quaternion& q2) const
	{
		return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
	}

	void Math::RotationAroundAxis(Quaternion& res, const Vector3Float& v, std::float_t rad) const
	{
		Vector3Float nrm = v;
		Normalize(nrm);

		const auto sinHalf = std::sinf(rad * 0.5f);
		const auto cosHalf = std::cosf(rad * 0.5f);

		res.x = nrm.x * sinHalf;
		res.y = nrm.y * sinHalf;
		res.z = nrm.z * sinHalf;
		res.w = cosHalf;
	}

	void Math::Multiply(Quaternion& res, const Quaternion& q1, const Quaternion& q2) const
	{
		res.x = q1.w* q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
		res.y = q1.w* q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z;
		res.z = q1.w* q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x;
		res.w = q1.w* q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
	}

	void Math::Inverse(Quaternion& res, const Quaternion& q) const
	{
		const auto sqrMag = Dot(q, q);
		res = q;
		ConjugateInPlace(res);

		const auto invSqrMag = 1.0f / sqrMag;
		res.x *= invSqrMag;
		res.y *= invSqrMag;
		res.z *= invSqrMag;
		res.w *= invSqrMag;
	}

	void Math::ConjugateInPlace(Quaternion& q)
	{
		q.x = -q.x;
		q.y = -q.y;
		q.z = -q.z;
	}

	void Math::FetchLocalToWorldSpaceMatrix(Matrix4x4& res, const std::shared_ptr<Node>& node) const
	{
		TRS(res, node->localTransform);
		auto currentNode = node->GetParent();

		while (currentNode)
		{
			Matrix4x4 currentNodeTrs {};
			TRS(currentNodeTrs, currentNode->localTransform);

			Matrix4x4 tmp = res;
			Multiply(res, currentNodeTrs, tmp);

			currentNode = currentNode->GetParent();
		}
	}
}