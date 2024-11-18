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
	private:
		std::shared_ptr<AppContext> _context;
	};

	class UserApp : public App
	{
	public:
		UserApp(const std::shared_ptr<AppContext>& context);
	};

	class RootApp : public App
	{
	protected:
		RootApp(const std::shared_ptr<AppContext>& context, const std::shared_ptr<UserApp>& userApp);
	public:
		void Initialize() override;
		void OnPause() override;
		void OnResume() override;
	protected:
		std::shared_ptr<UserApp> _userApp;
	};
}
