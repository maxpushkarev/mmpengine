#pragma once
#include <memory>
#include <Core/App.hpp>

namespace MMPEngine::Frontend
{
	class App final
	{
	public:
		static std::shared_ptr<Core::BaseRootApp> BuildRootApp(const Core::AppContext::Settings& appContextSettings, const std::shared_ptr<Core::UserApp>& userApp);
	};
}
