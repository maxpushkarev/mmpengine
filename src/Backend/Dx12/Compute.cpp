#include <Backend/Dx12/Compute.hpp>
#include <Backend/Dx12/Material.hpp>
#include <cassert>

namespace MMPEngine::Backend::Dx12
{
	DirectComputeJob::DirectComputeJob(const std::shared_ptr<Core::ComputeMaterial>& material) : Core::DirectComputeJob(material)
	{
	}

	std::shared_ptr<Core::BaseTask> DirectComputeJob::CreateInitializationTask()
	{
		return Core::BaseTask::kEmpty;
	}

	std::shared_ptr<Core::TaskWithInternalContext<Core::DirectComputeContext>> DirectComputeJob::CreateExecutionTask()
	{
		return nullptr;
	}

}
