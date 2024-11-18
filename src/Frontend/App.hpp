#pragma once
#include <memory>
#include <Core/App.hpp>

namespace MMPEngine::Frontend
{
	class App final
	{
	public:
		static std::shared_ptr<Core::App> BuildRootApp(const Core::AppContextSettings& appContextSettings, const std::shared_ptr<Core::UserApp>& userApp);
	};
}
