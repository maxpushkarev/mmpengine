#include <Core/Math.hpp>
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
	std::float_t Math::DeterminantInternal<Matrix4x4>(const Matrix4x4& m)
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
	std::float_t Math::DeterminantInternal<Matrix3x3>(const Matrix3x3& m)
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
	std::float_t Math::DeterminantInternal<Matrix2x2>(const Matrix2x2& m)
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
		assert(std::abs(det) >= _minDetAbs);
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

				const auto cofactor = sign * DeterminantInternal(tmp);
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
}