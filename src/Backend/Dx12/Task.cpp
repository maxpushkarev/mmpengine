#include <Backend/Dx12/Task.hpp>

namespace MMPEngine::Backend::Dx12
{
	Task<void>::Task() = default;

	void BindDescriptorHeapsTaskContext::FillDescriptors(const std::shared_ptr<AppContext>& ac)
	{
		descriptorHeaps.clear();
		descriptorHeaps.push_back(ac->cbvSrvUavShaderVisibleHeap);
		descriptorHeaps.push_back(ac->dsvHeap);
		descriptorHeaps.push_back(ac->rtvHeap);
	}

	BindDescriptorHeapsTask::BindDescriptorHeapsTask(const std::shared_ptr<BindDescriptorHeapsTaskContext>& ctx) : Task(ctx)
	{
	}

	void BindDescriptorHeapsTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		_nativeHeaps.clear();

		for(const auto& h : GetTaskContext()->descriptorHeaps)
		{
			h->CollectNativeBlocks(_nativeHeaps);
		}

		_specificStreamContext->PopulateCommandsInList()->SetDescriptorHeaps(
			static_cast<std::uint32_t>(_nativeHeaps.size()),
			_nativeHeaps.data()
		);
	}

}