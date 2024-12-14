#include <Core/Math.hpp>

namespace MMPEngine::Core
{
	Math::Math() = default;
	Math::~Math() = default;

	inline std::float_t Math::ConvertDegreesToRadians(std::float_t degrees)
	{
		return degrees * _deg2Rad;
	}
}