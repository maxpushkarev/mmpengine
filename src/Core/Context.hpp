#pragma once
#include <memory>
#include <Core/Base.hpp>
#include <Core/Logger.hpp>
#include <Core/Input.hpp>

#ifdef MMPENGINE_WIN
#include <Windows.h>
#endif

namespace MMPEngine::Core
{
	class Context
	{
	protected:
		Context();
	public:
		class Properties
		{
		public:
			Properties();
			Properties(const Properties&) = delete;
			Properties(Properties&&) noexcept = delete;
			Properties& operator=(const Properties&) = delete;
			Properties& operator=(Properties&&) noexcept = delete;
			virtual ~Properties();
		};
		Context(const Context&) = delete;
		Context(Context&&) noexcept = delete;
		Context& operator=(const Context&) = delete;
		Context& operator=(Context&&) noexcept = delete;
		virtual ~Context();

		std::shared_ptr<Properties> properties;
	};

#ifdef MMPENGINE_WIN
typedef HWND NativeWindow;
#endif

	class AppContext : public Context
	{
	public:
		struct Settings final
		{
			bool isDebug;
			BackendType backend;
		};
	protected:
		AppContext(const Settings&);
	public:
		const Settings settings;
		Vector2Uint windowSize;
		const PlatformType platform;
		std::unique_ptr<BaseLogger> logger;
		std::unique_ptr<Input> input;
#ifdef MMPENGINE_WIN
		NativeWindow nativeWindow;
#endif
	};

	class StreamContext : public Context
	{
	};

	class TaskContext : public Context
	{
	};
}
