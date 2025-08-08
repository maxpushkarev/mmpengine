#pragma once
#include <memory>
#include <Core/Base.hpp>
#include <Core/Math.hpp>

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

#ifdef MMPENGINE_MAC
typedef void* NativeWindow;
#endif

	class GlobalContext : public Context
	{
	public:
		struct Settings final
		{
			bool isDebug;
			BackendType backend;
		};
	protected:
		GlobalContext(const Settings&, std::unique_ptr<Core::Math>&&);
	public:
		const Settings settings;
		Vector2Uint windowSize;
		std::uint32_t screenRefreshRate;
		const PlatformType platform;
		const std::unique_ptr<Math> math;
#ifdef MMPENGINE_WIN
		NativeWindow nativeWindow = nullptr;
#endif
#ifdef MMPENGINE_MAC
        NativeWindow nativeWindow = nullptr;
#endif
	};

	class StreamContext : public Context
	{
	};

	class TaskContext : public Context
	{
	};

	template<typename TTaskContext>
	class TaskContextHolder
	{
		static_assert(std::is_base_of_v<TaskContext, TTaskContext>, "TTaskContext must be derived from TaskContext");
	protected:
		TaskContextHolder(const std::shared_ptr<TTaskContext>& taskContext);
	public:
		std::shared_ptr<TTaskContext> GetTaskContext() const;
	private:
		std::shared_ptr<TTaskContext> _taskContext;
	};

	template <typename TTaskContext>
	inline TaskContextHolder<TTaskContext>::TaskContextHolder(const std::shared_ptr<TTaskContext>& taskContext) : _taskContext(taskContext)
	{
	}

	template <typename TTaskContext>
	std::shared_ptr<TTaskContext> TaskContextHolder<TTaskContext>::GetTaskContext() const
	{
		return _taskContext;
	}

	template<>
	class TaskContextHolder<void>
	{
	};
}
