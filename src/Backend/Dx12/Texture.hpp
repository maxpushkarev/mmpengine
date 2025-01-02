#pragma once
#include <Core/Texture.hpp>

namespace MMPEngine::Backend::Dx12
{
	class DepthStencilTargetTexture final : public Core::DepthStencilTargetTexture
	{
	public:
		DepthStencilTargetTexture(const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	};
}