#pragma once
#include <Core/Base.h>

namespace MMPEngine::Core
{
	struct AppContextSettings final
	{
		bool isDebug;
		BackendType backend;
	};

	class AppContext
	{
	public:
		AppContext(const AppContextSettings&);
		AppContext(const AppContext&) = delete;
		AppContext(AppContext&&) noexcept = delete;
		AppContext& operator=(const AppContext&) = delete;
		AppContext& operator=(AppContext&&) noexcept = delete;
		virtual ~AppContext();

		const AppContextSettings baseSettings;
		Vector2Uint windowSize;
		const PlatformType platform;
	};
}
