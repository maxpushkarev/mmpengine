#pragma once
#include <Core/Compute.hpp>

namespace MMPEngine::Frontend
{
	class DirectComputeJob final : public Core::DirectComputeJob
	{
	public:
		DirectComputeJob(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::shared_ptr<Core::ComputeMaterial>& material);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::ContextualTask<Core::DirectComputeContext>> CreateExecutionTask() override;
	private:
		std::shared_ptr<Core::DirectComputeJob> _impl;
	};
}