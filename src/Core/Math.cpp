#include <Core/Math.hpp>

namespace MMPEngine::Core
{
	Math::Math() = default;
	Math::~Math() = default;

	inline std::float_t Math::ConvertDegreesToRadians(std::float_t degrees)
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

	void Math::TRS(Matrix4x4& matrix, const Transform& transform) const
	{
	}
}