#pragma once
#include <Core/Compute.hpp>

namespace MMPEngine::Backend::Dx12
{
	class DirectComputeJob final : public Core::DirectComputeJob
	{
	public:
		DirectComputeJob(const std::shared_ptr<Core::ComputeMaterial>& material);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::TaskWithInternalContext<Core::DirectComputeContext>> CreateExecutionTask() override;
	};
}