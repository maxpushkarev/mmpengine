#include <Backend/Dx12/Material.hpp>

namespace MMPEngine::Backend::Dx12
{
	ComputeMaterial::ComputeMaterial(const std::shared_ptr<Core::ComputeShader>& computeShader) : Core::ComputeMaterial(computeShader)
	{
	}

	std::shared_ptr<Core::BaseTask> ComputeMaterial::CreateTaskForApply()
	{
		return Core::BaseTask::kEmpty;
	}

	std::shared_ptr<Core::BaseTask> ComputeMaterial::CreateTaskForUpdateParametersInternal()
	{
		return Core::BaseTask::kEmpty;
	}

}