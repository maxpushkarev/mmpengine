#pragma once
#include <Core/Screen.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Screen final : public Core::Screen
	{
	public:
		Screen(const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateTaskToUpdate() override;
		std::shared_ptr<Core::BaseTask> CreateTaskToSwapBuffer() override;
	};
}