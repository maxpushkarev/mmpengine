#pragma once
#include <Core/Context.hpp>

namespace MMPEngine::Backend::Dx12
{
	class AppContext : Core::AppContext
	{
	public:
		AppContext(const Core::AppContextSettings& s);
	};
}
