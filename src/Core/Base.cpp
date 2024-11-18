#include <Core/Base.hpp>

namespace MMPEngine::Core
{
	UnsupportedException::UnsupportedException(const std::string& err) : std::runtime_error(err)
	{
	}
}