#include <Backend/Dx12/Material.hpp>

namespace MMPEngine::Backend::Dx12
{
	ComputeMaterial::ComputeMaterial(const std::shared_ptr<Core::ComputeShader>& computeShader) : Core::ComputeMaterial(computeShader)
	{
	}

	std::shared_ptr<Core::BaseTask> ComputeMaterial::CreateTaskForApply()
	{
		const auto ctx = std::make_shared<ApplyMaterialTaskContext>();
		ctx->materialImplPtr = std::dynamic_pointer_cast<MaterialImpl<Core::ComputeMaterial>>(shared_from_this());
		return std::make_shared<ApplyParametersTask>(ctx);
	}

	std::shared_ptr<Core::BaseTask> ComputeMaterial::CreateTaskForUpdateParametersInternal()
	{
		const auto ctx = std::make_shared<ParametersUpdatedTaskContext>();
		ctx->materialImplPtr = std::dynamic_pointer_cast<MaterialImpl<Core::ComputeMaterial>>(shared_from_this());
		ctx->params = &_params;
		return std::make_shared<ParametersUpdatedTask>(ctx);
	}

}