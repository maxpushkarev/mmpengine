#pragma once
#include <Core/Screen.hpp>

namespace MMPEngine::Frontend
{
	class Screen final : public Core::Screen
	{
	public:
		Screen(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreatePresentationTask() override;
		std::shared_ptr<Core::BaseTask> CreateStartFrameTask() override;
		std::shared_ptr<Core::ColorTargetTexture> GetBackBuffer() const override;
	protected:
		std::shared_ptr<Core::BaseTask> CreateInitializationTaskInternal() override;
		std::shared_ptr<Core::BaseTask> CreatePresentationTaskInternal() override;
		std::shared_ptr<Core::BaseTask> CreateStartFrameTaskInternal() override;
	private:
		std::shared_ptr<Core::Screen> _impl;
	};
}