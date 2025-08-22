#include <Frontend/Texture.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Texture.hpp>
#endif

#ifdef MMPENGINE_BACKEND_VULKAN
#include <Backend/Vulkan/Texture.hpp>
#endif

#ifdef MMPENGINE_BACKEND_METAL
#include <Backend/Metal/Texture.hpp>
#endif

namespace MMPEngine::Frontend
{
	DepthStencilTargetTexture::DepthStencilTargetTexture(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings) : Core::DepthStencilTargetTexture(settings)
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::DepthStencilTargetTexture>(this->_settings);
#else
			throw Core::UnsupportedException("unable to create depth/stencil texture for DX12 backend");
#endif
		}

		if (globalContext->settings.backend == Core::BackendType::Vulkan)
		{
#ifdef MMPENGINE_BACKEND_VULKAN
			_impl = std::make_shared<Backend::Vulkan::DepthStencilTargetTexture>(this->_settings);
#else
			throw Core::UnsupportedException("unable to create depth/stencil texture for Vulkan backend");
#endif
		}
        
        if (globalContext->settings.backend == Core::BackendType::Metal)
        {
#ifdef MMPENGINE_BACKEND_METAL
            _impl = std::make_shared<Backend::Metal::DepthStencilTargetTexture>(this->_settings);
#else
            throw Core::UnsupportedException("unable to create depth/stencil texture for Metal backend");
#endif
        }
	}

	std::shared_ptr<Core::BaseTask> DepthStencilTargetTexture::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();
	}

	std::shared_ptr<Core::BaseTexture> DepthStencilTargetTexture::GetUnderlyingTexture()
	{
		return _impl->GetUnderlyingTexture();
	}

}
