#include <Backend/Dx12/Texture.hpp>

namespace MMPEngine::Backend::Dx12
{
	DepthStencilTargetTexture::DepthStencilTargetTexture(const Settings& settings) : Core::DepthStencilTargetTexture(settings)
	{
	}

	std::shared_ptr<Core::BaseTask> DepthStencilTargetTexture::CreateInitializationTask()
	{
		return Core::BaseTask::kEmpty;
	}
}