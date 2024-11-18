#pragma once
#include <memory>
#include <Core/Context.hpp>

namespace MMPEngine::Frontend
{
	class AppContext final
	{
	public:
		static std::shared_ptr<Core::AppContext> BuildAppContext(const Core::AppContextSettings& appContextSettings);
	};
}
