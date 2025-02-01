#pragma once
#include <Core/Task.hpp>
#include <Backend/Vulkan/Context.hpp>

namespace MMPEngine::Backend::Vulkan
{
	template<typename TTaskContext>
	class Task : public Core::ContextHolderTask<GlobalContext, StreamContext, TTaskContext>
	{
	protected:
		Task(const std::shared_ptr<TTaskContext>& taskContext);
	};

	template<>
	class Task<void> : public Core::ContextHolderTask<GlobalContext, StreamContext, void>
	{
	protected:
		Task();
	};

	template <typename TTaskContext>
	Task<TTaskContext>::Task(const std::shared_ptr<TTaskContext>& taskContext) : Core::ContextHolderTask<GlobalContext, StreamContext, TTaskContext>(taskContext)
	{
	}
}