#pragma once
#include <memory>
#include <Core/Base.hpp>

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
		class CustomProperties
		{
		public:
			CustomProperties();
			CustomProperties(const CustomProperties&) = delete;
			CustomProperties(CustomProperties&&) noexcept = delete;
			CustomProperties& operator=(const CustomProperties&) = delete;
			CustomProperties& operator=(CustomProperties&&) noexcept = delete;
			virtual ~CustomProperties();
		};
		AppContext(const AppContextSettings&);
		AppContext(const AppContext&) = delete;
		AppContext(AppContext&&) noexcept = delete;
		AppContext& operator=(const AppContext&) = delete;
		AppContext& operator=(AppContext&&) noexcept = delete;
		virtual ~AppContext();

		const AppContextSettings settings;
		Vector2Uint windowSize;
		const PlatformType platform;
		std::shared_ptr<CustomProperties> customProps;
	};
}
