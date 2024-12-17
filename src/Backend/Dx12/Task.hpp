#pragma once
#include <Core/Task.hpp>
#include <Backend/Dx12/Context.hpp>

namespace MMPEngine::Backend::Dx12
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

	class BindDescriptorHeapsTaskContext : public Core::TaskContext
	{
	public:
		std::vector<std::shared_ptr<Dx12::BaseDescriptorHeap>> descriptorHeaps;
		void FillDescriptors(const std::shared_ptr<GlobalContext>& ac);
	};

	class BindDescriptorHeapsTask : public Task<BindDescriptorHeapsTaskContext>
	{
	public:
		BindDescriptorHeapsTask(const std::shared_ptr<BindDescriptorHeapsTaskContext>& ctx);
		void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
	private:
		std::vector<ID3D12DescriptorHeap*> _nativeHeaps;
	};

	template <typename TTaskContext>
	Task<TTaskContext>::Task(const std::shared_ptr<TTaskContext>& taskContext) : Core::ContextHolderTask<GlobalContext, StreamContext, TTaskContext>(taskContext)
	{
	}


}
