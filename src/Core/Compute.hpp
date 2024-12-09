#pragma once
#include <Core/Job.hpp>
#include <Core/Material.hpp>

namespace MMPEngine::Core
{
	class DirectComputeContext : public TaskContext
	{
	public:
		Core::Vector3Uint dimensions {1,1,1};
	};

	class BaseComputeJob
	{
	protected:
		BaseComputeJob(const std::shared_ptr<ComputeMaterial>& material);
		std::shared_ptr<ComputeMaterial> _material;
	};

	class DirectComputeJob : public BaseComputeJob, public Job<DirectComputeContext>
	{
	protected:
		DirectComputeJob(const std::shared_ptr<ComputeMaterial>& material);
	};
}
