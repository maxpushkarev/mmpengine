#include <Backend/Dx12/Task.hpp>

namespace MMPEngine::Backend::Dx12
{
	Task::Task() = default;

	void BindDescriptorHeapsTaskContext::FillDescriptors(const std::shared_ptr<AppContext>& ac)
	{
		descriptorHeaps.clear();
		descriptorHeaps.push_back(ac->cbvSrvUavShaderVisibleHeap);
		descriptorHeaps.push_back(ac->dsvHeap);
		descriptorHeaps.push_back(ac->rtvHeap);
	}

	BindDescriptorHeapsTask::BindDescriptorHeapsTask(const std::shared_ptr<BindDescriptorHeapsTaskContext>& ctx) : ContextualTask(ctx)
	{
	}

	void BindDescriptorHeapsTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
	}

	void BindDescriptorHeapsTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		_nativeHeaps.clear();

		for(const auto& h : _taskContext->descriptorHeaps)
		{
			h->CollectNativeBlocks(_nativeHeaps);
		}

		_specificStreamContext->PopulateCommandsInList()->SetDescriptorHeaps(
			static_cast<std::uint32_t>(_nativeHeaps.size()),
			_nativeHeaps.data()
		);
	}

	void BindDescriptorHeapsTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

}