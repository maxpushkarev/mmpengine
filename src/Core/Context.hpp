#pragma once
#include <memory>
#include <Core/Base.hpp>
#include <Core/Logger.hpp>
#include <Core/Input.hpp>

namespace MMPEngine::Core
{
	class Context
	{
	protected:
		Context();
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
		Context(const Context&) = delete;
		Context(Context&&) noexcept = delete;
		Context& operator=(const Context&) = delete;
		Context& operator=(Context&&) noexcept = delete;
		virtual ~Context();
	};

	struct AppContextSettings final
	{
		bool isDebug;
		BackendType backend;
	};

	class AppContext : public Context
	{
	protected:
		AppContext(const AppContextSettings&);
	public:
		const AppContextSettings settings;
		Vector2Uint windowSize;
		const PlatformType platform;
		std::shared_ptr<CustomProperties> customProps;
		std::unique_ptr<BaseLogger> logger;
		std::unique_ptr<Input> input;
	};

	class StreamContext : public Context
	{
		
	};

	class TaskContext : public Context
	{

	};
}
