#include <Core/Text.hpp>

namespace MMPEngine::Core
{
	Text::Text() = default;

	std::string Text::CombineToStringInternal(const std::stringstream& ss)
	{
		return ss.str();
	}

	std::string Text::ToString(BackendType backend)
	{
		switch (backend)
		{
		case Core::BackendType::Dx12:
			return "Dx12";
		case Core::BackendType::Vulkan:
			return "Vulkan";
		case Core::BackendType::Metal:
			return "Metal";
		}

		throw UnsupportedException {"unknown backend type"};
	}

}
