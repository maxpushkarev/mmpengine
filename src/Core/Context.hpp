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
		std::shared_ptr<CustomProperties> customProps;
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
