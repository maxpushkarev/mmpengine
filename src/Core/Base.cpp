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

	bool Matrix4x4::operator==(const Matrix4x4& rhs) const
	{
		for(std::size_t i = 0; i < 4; ++i)
		{
			for(std::size_t j = 0; j < 4; ++j)
			{
				if(std::abs(m[i][j] - rhs.m[i][j]) > Constants::kFloatEps)
				{
					return false;
				}
			}
		}
		return true;
	}

	bool Matrix4x4::operator!=(const Matrix4x4& rhs) const
	{
		return !operator==(rhs);
	}
}