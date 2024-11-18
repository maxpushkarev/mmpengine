#include <Core/App.hpp>

namespace MMPEngine::Core
{
	App::App(const std::shared_ptr<AppContext>& context) : _context(context)
	{
	}
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

    std::shared_ptr<AppContext> App::GetContext() const
    {
        return _context;
    }

	UserApp::UserApp(const std::shared_ptr<AppContext>& context) : App(context)
	{
	}

	RootApp::RootApp(const std::shared_ptr<AppContext>& context, const std::shared_ptr<UserApp>& userApp) : App(context), _userApp(userApp)
	{
	}

	void RootApp::Initialize()
	{
		App::Initialize();
		if(_userApp)
		{
			_userApp->Initialize();
		}
	}

	void RootApp::OnPause()
	{
		App::OnPause();
		if(_userApp)
		{
			_userApp->OnPause();
		}
	}

	void RootApp::OnResume()
	{
		App::OnResume();
		if(_userApp)
		{
			_userApp->OnResume();
		}
	}
}