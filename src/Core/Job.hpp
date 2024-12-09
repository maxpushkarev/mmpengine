#pragma once
#include <Core/Base.hpp>
#include <Core/Task.hpp>

namespace MMPEngine::Core
{
	class BaseJob : public IInitializationTaskSource, public std::enable_shared_from_this<BaseJob>
	{
	};

	template<typename TExecutionTaskContext>
	class Job : public BaseJob
	{
	public:
		virtual std::shared_ptr<TaskWithContext<TExecutionTaskContext>> CreateExecutionTask() = 0;
	};

	template<>
	class Job<void> : public BaseJob
	{
	public:
		virtual std::shared_ptr<BaseTask> CreateExecutionTask() = 0;
	};
}
