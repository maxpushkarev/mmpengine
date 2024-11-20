#pragma once
#include <Core/Context.hpp>

namespace MMPEngine::Backend::Dx12
{
	class AppContext : public Core::AppContext
	{
	public:
		AppContext(const Core::AppContext::Settings& s);
	};
}
