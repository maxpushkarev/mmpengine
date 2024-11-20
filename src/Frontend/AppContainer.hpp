#pragma once
#include <memory>
#include <Core/App.hpp>

namespace MMPEngine::Frontend
{
	class AppContainer
	{
	protected:
		AppContainer(const std::shared_ptr<Core::BaseRootApp>& app);
	public:
		AppContainer(const AppContainer&) = delete;
		AppContainer(AppContainer&&) noexcept = delete;
		AppContainer& operator=(const AppContainer&) = delete;
		AppContainer& operator=(AppContainer&&) noexcept = delete;
		virtual ~AppContainer();
	protected:
		std::shared_ptr<Core::App> _app;
		std::shared_ptr<Core::AppInputController> _inputController;
	};
}
