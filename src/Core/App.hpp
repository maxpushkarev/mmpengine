#pragma once
#include <memory>
#include <Core/Context.hpp>
#include <Core/Stream.hpp>

namespace MMPEngine::Core
{
	class BaseRootApp;

	class AppInputController final : public IInputController
	{
	public:
		AppInputController(const std::shared_ptr<BaseRootApp>&);
		void ClearAll() override;
		void ClearInstantEvents() override;
		void SetButtonPressedStatus(KeyButton, bool) override;
		void SetButtonPressedStatus(MouseButton, bool) override;
		void UpdateMouseNormalizedPosition(const Vector2Float&) override;
	private:
		std::shared_ptr<GlobalContext> _globalContext;
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
		virtual void OnNativeWindowUpdated();
		virtual void OnPause();
		virtual void OnResume();
		virtual void OnUpdate(std::float_t dt);
		virtual void OnRender();
		virtual std::shared_ptr<GlobalContext> GetContext() const = 0;
		virtual std::shared_ptr<BaseStream> GetDefaultStream() const = 0;
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
		std::shared_ptr<GlobalContext> GetContext() const override;
		std::shared_ptr<BaseStream> GetDefaultStream() const override;
	private:
		void JoinToRootApp(const std::shared_ptr<BaseRootApp>& root);
		std::weak_ptr<BaseRootApp> _rootApp;
	};

	template<typename TRootContext>
	class RootApp : public BaseRootApp
	{
		static_assert(std::is_base_of_v<GlobalContext, TRootContext>, "TRootContext must be inherited from Core::GlobalContext");
	protected:
		RootApp(const std::shared_ptr<TRootContext>& context);
	public:
		RootApp(const RootApp&) = delete;
		RootApp(RootApp&&) noexcept = delete;
		RootApp& operator=(const RootApp&) = delete;
		RootApp& operator=(RootApp&&) noexcept = delete;
	protected:
		~RootApp() override;
	public:
		std::shared_ptr<GlobalContext> GetContext() const override;
		std::shared_ptr<BaseStream> GetDefaultStream() const override;
	protected:
		std::shared_ptr<TRootContext> _rootContext;
		std::shared_ptr<BaseStream> _defaultStream;
	};

	template<typename TRootContext>
	inline RootApp<TRootContext>::RootApp(const std::shared_ptr<TRootContext>& context) : _rootContext(context)
	{
	}
	
	template<typename TRootContext>
	inline RootApp<TRootContext>::~RootApp()
	{
		_defaultStream->SubmitAndWait();
	}

	template<typename TRootContext>
	inline std::shared_ptr<GlobalContext> RootApp<TRootContext>::GetContext() const
	{
		return _rootContext;
	}
	template<typename TRootContext>
	inline std::shared_ptr<BaseStream> RootApp<TRootContext>::GetDefaultStream() const
	{
		return _defaultStream;
	}
}
