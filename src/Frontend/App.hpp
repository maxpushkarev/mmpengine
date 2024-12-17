#pragma once
#include <memory>
#include <Core/App.hpp>
#include <Core/Math.hpp>

namespace MMPEngine::Frontend
{
	class App final
	{
	public:
		static std::shared_ptr<Core::BaseRootApp> BuildRootApp(const Core::GlobalContext::Settings& globalContextSettings, const std::shared_ptr<Core::UserApp>& userApp, std::unique_ptr<Core::Math>&& math, std::unique_ptr<Core::BaseLogger>&& logger);
	};
}
