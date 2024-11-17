#pragma once
#include <memory>
#include <Core/Context.hpp>

namespace MMPEngine::Core
{
	class App
	{
	public:
		App(const std::shared_ptr<AppContext>& context);
		App(const App&) = delete;
		App(App&&) noexcept = delete;
		App& operator=(const App&) = delete;
		App& operator=(App&&) noexcept = delete;
		virtual ~App();

		virtual void Initialize();
		virtual void OnPause();
		virtual void OnResume();
	private:
		std::shared_ptr<AppContext> _context;
	};
}
