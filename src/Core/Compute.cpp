#include <Core/Compute.hpp>

namespace MMPEngine::Core
{
	DirectComputeJob::DirectComputeJob(const std::shared_ptr<ComputeMaterial>& material) : _material(material)
	{
	}
}