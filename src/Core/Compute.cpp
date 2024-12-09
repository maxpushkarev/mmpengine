#include <Core/Compute.hpp>

namespace MMPEngine::Core
{
	BaseComputeJob::BaseComputeJob(const std::shared_ptr<ComputeMaterial>& material) : _material(material)
	{
	}

	DirectComputeJob::DirectComputeJob(const std::shared_ptr<ComputeMaterial>& material) : BaseComputeJob(material)
	{
	}
}