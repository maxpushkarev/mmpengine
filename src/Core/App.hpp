#pragma once
#include <memory>
#include <Core/Context.hpp>

namespace MMPEngine::Core
{
	class BaseRootApp;

	class AppInputController final : public IInputController
	{
	public:
		AppInputController(const std::shared_ptr<BaseRootApp>&);
		void ClearAll() override;
		void ClearEvents() override;
		void SetButtonPressedStatus(KeyButton, bool) override;
		void SetButtonPressedStatus(MouseButton, bool) override;
		void UpdateMouseNormalizedPosition(const Vector2Float&) override;
	private:
		std::shared_ptr<AppContext> _appContext;
	};

	class App : public std::enable_shared_from_this<App>
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
		BaseRootApp();
		void Initialize() override;
		void OnPause() override;
		void OnResume() override;
		std::shared_ptr<UserApp> _userApp;
	public:
		void Attach(const std::shared_ptr<UserApp>& userApp);
	};

	class UserApp : public App
	{
		friend class BaseRootApp;
	public:
		UserApp();
		std::shared_ptr<AppContext> GetContext() const override;
	private:
		void JoinToRootApp(const std::shared_ptr<BaseRootApp>& root);
		std::weak_ptr<BaseRootApp> _rootApp;
	};

	template<typename TRootContext>
	class RootApp : public BaseRootApp
	{
		static_assert(std::is_base_of_v<AppContext, TRootContext>, "TRootContext must be inherited from Core::AppContext");
	protected:
		RootApp(const std::shared_ptr<TRootContext>& context);
		std::shared_ptr<AppContext> GetContext() const override;
	protected:
		std::shared_ptr<TRootContext> _rootContext;
	};

	template<typename TRootContext>
	inline RootApp<TRootContext>::RootApp(const std::shared_ptr<TRootContext>& context) : _rootContext(context)
	{
	}
	
	template<typename TRootContext>
	inline std::shared_ptr<AppContext> RootApp<TRootContext>::GetContext() const
	{
		return _rootContext;
	}
}
