#include <Backend/Dx12/Screen.hpp>

namespace MMPEngine::Backend::Dx12
{
	Screen::Screen(const Settings& settings) : Core::Screen(settings)
	{
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateInitializationTask()
	{
		return Core::BaseTask::kEmpty;
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateTaskToUpdate()
	{
		return Core::BaseTask::kEmpty;
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateTaskToSwapBuffer()
	{
		return Core::BaseTask::kEmpty;
	}
}