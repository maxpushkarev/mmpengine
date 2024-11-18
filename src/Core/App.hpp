#pragma once
#include <memory>
#include <Core/Context.hpp>

namespace MMPEngine::Core
{
	class App
	{
	protected:
		App();
	public:
		App(const App&) = delete;
		App(App&&) noexcept = delete;
		App& operator=(const App&) = delete;
		App& operator=(App&&) noexcept = delete;
		virtual ~App();

		virtual void Initialize();
		virtual void OnPause();
		virtual void OnResume();
		virtual std::shared_ptr<AppContext> GetContext() const = 0;
	};

	class UserApp;

	class BaseRootApp : public App
	{
	protected:
		BaseRootApp(const std::shared_ptr<UserApp>& userApp);
		void Initialize() override;
		void OnPause() override;
		void OnResume() override;
		std::shared_ptr<UserApp> _userApp;
	};

	class UserApp : public App
	{
	public:
		UserApp();
		std::shared_ptr<AppContext> GetContext() const override;
	};

	template<typename TRootContext>
	class RootApp : public BaseRootApp
	{
		static_assert(std::is_base_of_v<AppContext, TRootContext>, "TRootContext must be inherited from Core::AppContext");
	protected:
		RootApp(const std::shared_ptr<TRootContext>& context, const std::shared_ptr<UserApp>& userApp);
		std::shared_ptr<AppContext> GetContext() const override;
	protected:
		std::shared_ptr<TRootContext> _rootContext;
	};

	template<typename TRootContext>
	inline RootApp<TRootContext>::RootApp(const std::shared_ptr<TRootContext>& context, const std::shared_ptr<UserApp>& userApp) : BaseRootApp(userApp), _rootContext(context)
	{
	}
	
	template<typename TRootContext>
	inline std::shared_ptr<AppContext> RootApp<TRootContext>::GetContext() const
	{
		return _rootContext;
	}
}
