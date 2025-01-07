#include <Core/Screen.hpp>
#include <cassert>

namespace MMPEngine::Core
{
	Screen::Screen(const Settings& settings) : _settings(settings), _streamPtr(nullptr)
	{
	}

	const Screen::Settings& Screen::GetSettings() const
	{
		return _settings;
	}

	Screen::StreamValidationTask::StreamValidationTask(const std::shared_ptr<TaskContext>& ctx) : ContextualTask<MMPEngine::Core::Screen::TaskContext>(ctx)
	{
	}

	void Screen::StreamValidationTask::OnScheduled(const std::shared_ptr<BaseStream>& stream)
	{
		ContextualTask::OnScheduled(stream);

		const auto screen = GetTaskContext()->screen;
		if(screen->_streamPtr)
		{
			assert(screen->_streamPtr == stream.get());
		}
		else
		{
			screen->_streamPtr = stream.get();
		}
	}

}