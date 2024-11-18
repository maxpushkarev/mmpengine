#pragma once
#include <memory>
#include <Core/Context.hpp>

namespace MMPEngine::Core
{
	class App
	{
	protected:
		App(const std::shared_ptr<AppContext>& context);
	public:
		App(const App&) = delete;
		App(App&&) noexcept = delete;
		App& operator=(const App&) = delete;
		App& operator=(App&&) noexcept = delete;
		virtual ~App();

		virtual void Initialize();
		virtual void OnPause();
		virtual void OnResume();
		std::shared_ptr<AppContext> GetContext() const;
	protected:
		std::shared_ptr<AppContext> _context;
	};

	class UserApp : public App
	{
	public:
		UserApp(const std::shared_ptr<AppContext>& context);
	};

	template<typename TRootContext>
	class RootApp : public App
	{
		static_assert(std::is_base_of_v<AppContext, TRootContext>, "TRootContext must be inherited from Core::AppContext");
	protected:
		RootApp(const std::shared_ptr<TRootContext>& context, const std::shared_ptr<UserApp>& userApp);
	public:
		void Initialize() override;
		void OnPause() override;
		void OnResume() override;
	protected:
		std::shared_ptr<TRootContext> _rootContext;
		std::shared_ptr<UserApp> _userApp;
	};

	template<typename TRootContext>
	RootApp<TRootContext>::RootApp(const std::shared_ptr<TRootContext>& context, const std::shared_ptr<UserApp>& userApp) : App(context), _userApp(userApp)
	{
	}

	template<typename TRootContext>
	void RootApp<TRootContext>::Initialize()
	{
		App::Initialize();
		if (_userApp)
		{
			_userApp->Initialize();
		}
	}

	template<typename TRootContext>
	void RootApp<TRootContext>::OnPause()
	{
		App::OnPause();
		if (_userApp)
		{
			_userApp->OnPause();
		}
	}

	template<typename TRootContext>
	void RootApp<TRootContext>::OnResume()
	{
		App::OnResume();
		if (_userApp)
		{
			_userApp->OnResume();
		}
	}
}
