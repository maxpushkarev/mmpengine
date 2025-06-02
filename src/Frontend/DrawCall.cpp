#include <Frontend/DrawCall.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/DrawCall.hpp>
#endif

#ifdef MMPENGINE_BACKEND_VULKAN
#include <Backend/Vulkan/DrawCall.hpp>
#endif

namespace MMPEngine::Frontend
{
	Camera::DrawCallsJob::DrawCallsJob(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::shared_ptr<Core::Camera>& camera, std::vector<Item>&& items)
		: Core::Camera::DrawCallsJob(camera, std::move(items))
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::Camera::DrawCallsJob>(camera, std::move(_items));
#else
			throw Core::UnsupportedException("unable to create camera drawcalls job for DX12 backend");
#endif
		} else 	if (globalContext->settings.backend == Core::BackendType::Vulkan)
		{
#ifdef MMPENGINE_BACKEND_VULKAN
			_impl = std::make_shared<Backend::Vulkan::Camera::DrawCallsJob>(camera, std::move(_items));
#else
			throw Core::UnsupportedException("unable to create camera drawcalls job for Vulkan backend");
#endif
		}
	}

	std::shared_ptr<Camera::DrawCallsJob::Iteration> Camera::DrawCallsJob::BuildIteration(const Item& item) const
	{
		throw std::logic_error("impossible");
	}

	std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::CreateTaskForIterationsStart()
	{
		throw std::logic_error("impossible");
	}

	std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();	
	}

	std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::CreateExecutionTask()
	{
		return _impl->CreateExecutionTask();
	}
}