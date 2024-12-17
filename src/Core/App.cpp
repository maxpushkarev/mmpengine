#include <Core/App.hpp>

namespace MMPEngine::Core
{
	App::App() = default;
	App::~App() = default;

	void App::Initialize()
	{
	}

	void App::OnNativeWindowUpdated()
	{
	}

	void App::OnPause()
	{
	}

	void App::OnResume()
	{
	}

    void App::OnUpdate(std::float_t dt)
    {
    }

	void App::OnRender()
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
	std::shared_ptr<GlobalContext> UserApp::GetContext() const
	{
		if(const auto root = _rootApp.lock())
		{
			return root->GetContext();
		}

		return nullptr;
	}

	std::shared_ptr<BaseStream> UserApp::GetDefaultStream() const
	{
		if (const auto root = _rootApp.lock())
		{
			return root->GetDefaultStream();
		}

		return nullptr;
	}


	void UserApp::JoinToRootApp(const std::shared_ptr<BaseRootApp>& root)
	{
		_rootApp = root;
	}

    AppInputController::AppInputController(const std::shared_ptr<BaseRootApp>& baseRootApp) : _globalContext(baseRootApp->GetContext())
    {
    }

	void AppInputController::ClearAll()
	{
		_globalContext->input->ClearAll();
	}

	void AppInputController::ClearInstantEvents()
	{
		_globalContext->input->ClearInstantEvents();
	}

	void AppInputController::UpdateMouseNormalizedPosition(const Vector2Float& pos)
	{
		_globalContext->input->UpdateMouseNormalizedPosition(pos);
	}

	void AppInputController::SetButtonPressedStatus(KeyButton btn, bool status)
	{
		_globalContext->input->SetButtonPressedStatus(btn, status);
	}

	void AppInputController::SetButtonPressedStatus(MouseButton btn, bool status)
	{
		_globalContext->input->SetButtonPressedStatus(btn, status);
	}
}