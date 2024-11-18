#include <Core/Text.hpp>

namespace MMPEngine::Core
{
	Text::Text() = default;

	std::string Text::CombineToStringInternal(const std::stringstream& ss)
	{
		return ss.str();
	}
}