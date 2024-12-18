#include <Backend/Dx12/Task.hpp>

namespace MMPEngine::Backend::Dx12
{
	Task<void>::Task() = default;

	void BindDescriptorPoolsTaskContext::FillDescriptors(const std::shared_ptr<GlobalContext>& ac)
	{
		descriptorHeaps.clear();
		descriptorHeaps.push_back(ac->cbvSrvUavShaderVisibleDescPool);
		descriptorHeaps.push_back(ac->dsvDescPool);
		descriptorHeaps.push_back(ac->rtvDescPool);
	}

	BindDescriptorPoolsTask::BindDescriptorPoolsTask(const std::shared_ptr<BindDescriptorPoolsTaskContext>& ctx) : Task(ctx)
	{
	}

	void BindDescriptorPoolsTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
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