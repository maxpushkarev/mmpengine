#include <Frontend/Screen.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Screen.hpp>
#endif

namespace MMPEngine::Frontend
{
	Screen::Screen(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings) : Core::Screen(settings)
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::Screen>(settings);
#else
			throw Core::UnsupportedException("unable to create screen for DX12 backend");
#endif
		}
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateTaskToUpdate()
	{
		return _impl->CreateTaskToUpdate();
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateSyncTask()
	{
		return _impl->CreateSyncTask();
	}

}