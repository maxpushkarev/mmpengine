#include <Frontend/Screen.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Screen.hpp>
#endif

#ifdef MMPENGINE_BACKEND_VULKAN
#include <Backend/Vulkan/Screen.hpp>
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


		if (globalContext->settings.backend == Core::BackendType::Vulkan)
		{
#ifdef MMPENGINE_BACKEND_VULKAN
			_impl = std::make_shared<Backend::Vulkan::Screen>(settings);
#else
			throw Core::UnsupportedException("unable to create screen for Vulkan backend");
#endif
		}
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateTaskToSwapBuffer()
	{
		return _impl->CreateTaskToSwapBuffer();
	}

	std::shared_ptr<Core::ColorTargetTexture> Screen::GetBackBuffer() const
	{
		return _impl->GetBackBuffer();
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateInitializationTaskInternal()
	{
		throw std::logic_error {"impossible"};
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateTaskToSwapBufferInternal()
	{
		throw std::logic_error {"impossible"};
	}
}