#pragma once
#include <Core/Screen.hpp>

namespace MMPEngine::Frontend
{
	class Screen final : public Core::Screen
	{
	public:
		Screen(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateTaskToUpdate() override;
		std::shared_ptr<Core::BaseTask> CreateTaskToSwapBuffer() override;
		std::shared_ptr<Core::TargetTexture> GetBackBuffer(std::uint32_t index) const override;
		std::uint32_t GetCurrentBackBufferIndex() const override;
	private:
		std::shared_ptr<Core::Screen> _impl;
	};
}