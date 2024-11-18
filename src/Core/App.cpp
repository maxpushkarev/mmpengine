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

	BaseRootApp::BaseRootApp() = default;

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

	void BaseRootApp::Attach(const std::shared_ptr<UserApp>& userApp)
	{
		_userApp = userApp;
		_userApp->JoinToRootApp(std::dynamic_pointer_cast<BaseRootApp>(shared_from_this()));
	}

	UserApp::UserApp() = default;
	std::shared_ptr<AppContext> UserApp::GetContext() const
	{
		if(const auto root = _rootApp.lock())
		{
			return root->GetContext();
		}

		return nullptr;
	}

	void UserApp::JoinToRootApp(const std::shared_ptr<BaseRootApp>& root)
	{
		_rootApp = root;
	}

}