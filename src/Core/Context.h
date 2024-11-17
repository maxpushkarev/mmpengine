#pragma once
#include <Core/Base.h>

namespace MMPEngine::Core
{
	class AppContext
	{
	public:
		AppContext();
		AppContext(const AppContext&) = delete;
		AppContext(AppContext&&) noexcept = delete;
		AppContext& operator=(const AppContext&) = delete;
		AppContext& operator=(AppContext&&) noexcept = delete;
		virtual ~AppContext();

		Vector2Uint WindowSize;
	};
}
