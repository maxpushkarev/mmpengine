#include <Frontend/Texture.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Texture.hpp>
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