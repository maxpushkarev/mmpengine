#pragma once
#include <Core/Task.hpp>
#include <Backend/Dx12/Context.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Task : public Core::ExternalContextSpecificTask<AppContext, StreamContext>
	{
	protected:
		Task();
	};

	class BindDescriptorHeapsTaskContext : public Core::TaskContext
	{
	public:
		std::vector<std::shared_ptr<Dx12::BaseDescriptorHeap>> descriptorHeaps;
		void FillDescriptors(const std::shared_ptr<AppContext>& ac);
	};

	class BindDescriptorHeapsTask : public Task, public Core::ContextualTask<BindDescriptorHeapsTaskContext>
	{
	public:
		BindDescriptorHeapsTask(const std::shared_ptr<BindDescriptorHeapsTaskContext>& ctx);
		void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		void OnComplete(const std::shared_ptr<Core::BaseStream>& stream) override;
	private:
		std::vector<ID3D12DescriptorHeap*> _nativeHeaps;
	};
}
