#include <Core/App.hpp>

namespace MMPEngine::Core
{
	App::App() = default;
	App::~App() = default;

	void App::Initialize()
	{
	}

	void App::OnPause()
	{
	}

	void App::OnResume()
	{
	}

	BaseRootApp::BaseRootApp(const std::shared_ptr<UserApp>& userApp) : _userApp(userApp)
	{
	}

	void BaseRootApp::Initialize()
	{
		App::Initialize();
		if (_userApp)
		{
			_userApp->Initialize();
		}
	}

	void BaseRootApp::OnPause()
	{
		App::OnPause();
		if (_userApp)
		{
			_userApp->OnPause();
		}
	}

	void BaseRootApp::OnResume()
	{
		App::OnResume();
		if(_userApp)
		{
			_userApp->OnResume();
		}
	}

	UserApp::UserApp() = default;
	std::shared_ptr<AppContext> UserApp::GetContext() const
	{
		return nullptr;
	}

}