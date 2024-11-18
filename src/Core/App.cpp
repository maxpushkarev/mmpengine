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

    const AppContext* App::GetContext() const
    {
        return _context.get();
    }

	UserApp::UserApp(const std::shared_ptr<AppContext>& context) : App(context)
	{
	}

	RootApp::RootApp(const std::shared_ptr<AppContext>& context, const std::shared_ptr<UserApp>& userApp) : App(context), _userApp(userApp)
	{
	}

}