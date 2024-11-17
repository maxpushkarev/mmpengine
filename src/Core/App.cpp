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

}