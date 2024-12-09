#include <Frontend/Compute.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Compute.hpp>
#endif

namespace MMPEngine::Frontend
{
	DirectComputeJob::DirectComputeJob(const std::shared_ptr<Core::AppContext>& appContext, const std::shared_ptr<Core::ComputeMaterial>& material) : Core::DirectComputeJob(material)
	{
		if (appContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::DirectComputeJob>(material);
#else
			throw Core::UnsupportedException("unable to create compute job for DX12 backend");
#endif
		}
	}

	std::shared_ptr<Core::TaskWithContext<Core::DirectComputeContext>> DirectComputeJob::CreateExecutionTask()
	{
		return _impl->CreateExecutionTask();
	}

	std::shared_ptr<Core::BaseTask> DirectComputeJob::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();
	}


}