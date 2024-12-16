#include <Core/Base.hpp>

namespace MMPEngine::Core
{
	UnsupportedException::UnsupportedException(const std::string& err) : std::runtime_error(err)
	{
	}

    IInitializationTaskSource::IInitializationTaskSource() = default;
	IInitializationTaskSource::~IInitializationTaskSource() = default;

    INamed::INamed() = default;
	INamed::~INamed() = default;

	bool Vector2Float::operator!=(const Vector2Float& rhs) const
	{
		return !operator==(rhs);
	}

	bool Vector2Float::operator==(const Vector2Float& rhs) const
	{
		return std::abs(x - rhs.x) <= Constants::kFloatEps &&
				std::abs(y - rhs.y) <= Constants::kFloatEps;
	}

	bool Vector3Float::operator==(const Vector3Float& rhs) const
	{
		return std::abs(x - rhs.x) <= Constants::kFloatEps &&
			std::abs(y - rhs.y) <= Constants::kFloatEps &&
			std::abs(z - rhs.z) <= Constants::kFloatEps;
	}

	bool Vector3Float::operator!=(const Vector3Float& rhs) const
	{
		return !operator==(rhs);
	}

	bool Vector4Float::operator==(const Vector4Float& rhs) const
	{
		return std::abs(x - rhs.x) <= Constants::kFloatEps &&
			std::abs(y - rhs.y) <= Constants::kFloatEps &&
			std::abs(z - rhs.z) <= Constants::kFloatEps &&
			std::abs(w - rhs.w) <= Constants::kFloatEps;
	}

	bool Vector4Float::operator!=(const Vector4Float& rhs) const
	{
		return !operator==(rhs);
	}

	bool Vector2Uint::operator==(const Vector2Uint& rhs) const
	{
		return x == rhs.x && 
				y == rhs.y;
	}

	bool Vector2Uint::operator!=(const Vector2Uint& rhs) const
	{
		return !operator==(rhs);
	}

	bool Vector3Uint::operator==(const Vector3Uint& rhs) const
	{
		return x == rhs.x &&
			y == rhs.y &&
			z == rhs.z;
	}

	bool Vector3Uint::operator!=(const Vector3Uint& rhs) const
	{
		return !operator==(rhs);
	}

	bool Vector4Uint::operator==(const Vector4Uint& rhs) const
	{
		return x == rhs.x &&
			y == rhs.y &&
			z == rhs.z &&
			w == rhs.w;
	}

	bool Vector4Uint::operator!=(const Vector4Uint& rhs) const
	{
		return !operator==(rhs);
	}


	bool Vector2Int::operator==(const Vector2Int& rhs) const
	{
		return x == rhs.x &&
			y == rhs.y;
	}

	bool Vector2Int::operator!=(const Vector2Int& rhs) const
	{
		return !operator==(rhs);
	}

	bool Vector3Int::operator==(const Vector3Int& rhs) const
	{
		return x == rhs.x &&
			y == rhs.y &&
			z == rhs.z;
	}

	bool Vector3Int::operator!=(const Vector3Int& rhs) const
	{
		return !operator==(rhs);
	}

	bool Vector4Int::operator==(const Vector4Int& rhs) const
	{
		return x == rhs.x &&
			y == rhs.y &&
			z == rhs.z &&
			w == rhs.w;
	}

	bool Vector4Int::operator!=(const Vector4Int& rhs) const
	{
		return !operator==(rhs);
	}

	bool Quaternion::operator==(const Quaternion& rhs) const
	{
		return std::abs(x - rhs.x) <= Constants::kFloatEps &&
			std::abs(y - rhs.y) <= Constants::kFloatEps &&
			std::abs(z - rhs.z) <= Constants::kFloatEps &&
			std::abs(w - rhs.w) <= Constants::kFloatEps;
	}

	bool Quaternion::operator!=(const Quaternion& rhs) const
	{
		return !operator==(rhs);
	}

	inline bool MatrixEqualityCheck(std::size_t size, const std::float_t* lhs, const std::float_t* rhs)
	{
		for (std::size_t i = 0; i < size; ++i)
		{
			for (std::size_t j = 0; j < size; ++j)
			{
				const auto linear = i * size + j;

				if (std::abs(lhs[linear] - rhs[linear]) > Constants::kFloatEps)
				{
					return false;
				}
			}
		}
		return true;
	}

	bool Matrix4x4::operator==(const Matrix4x4& rhs) const
	{
		return MatrixEqualityCheck(4, &(m[0][0]), &(rhs.m[0][0]));
	}

	bool Matrix4x4::operator!=(const Matrix4x4& rhs) const
	{
		return !operator==(rhs);
	}

	bool Matrix3x3::operator==(const Matrix3x3& rhs) const
	{
		return MatrixEqualityCheck(3, &(m[0][0]), &(rhs.m[0][0]));
	}

	bool Matrix3x3::operator!=(const Matrix3x3& rhs) const
	{
		return !operator==(rhs);
	}

	bool Matrix2x2::operator==(const Matrix2x2& rhs) const
	{
		return MatrixEqualityCheck(2, &(m[0][0]), &(rhs.m[0][0]));
	}

	bool Matrix2x2::operator!=(const Matrix2x2& rhs) const
	{
		return !operator==(rhs);
	}

	bool Transform::operator==(const Transform& rhs) const
	{
		return (position == rhs.position) && (scale == rhs.scale) && (rotation == rhs.rotation);
	}

	bool Transform::operator!=(const Transform& rhs) const
	{
		return !operator==(rhs);
	}
}