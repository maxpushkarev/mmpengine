#pragma once
#include <Core/Material.hpp>

namespace MMPEngine::Backend::Dx12
{
	template<class TCoreMaterial>
	class MaterialImpl
	{
	};

	class ComputeMaterial final : public Core::ComputeMaterial, public MaterialImpl<Core::ComputeMaterial>
	{
	public:
		ComputeMaterial(const std::shared_ptr<Core::ComputeShader>& computeShader);
		std::shared_ptr<Core::BaseTask> CreateTaskForApply() override;
		std::shared_ptr<Core::BaseTask> CreateTaskForUpdateParametersInternal() override;
	};
}