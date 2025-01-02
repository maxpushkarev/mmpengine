#pragma once
#include <Core/Texture.hpp>

namespace MMPEngine::Frontend
{
	class DepthStencilTargetTexture : public Core::DepthStencilTargetTexture
	{
	public:
		DepthStencilTargetTexture(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<BaseTexture> GetUnderlyingTexture() override;
	private:
		std::shared_ptr<Core::DepthStencilTargetTexture> _impl;
	};
}