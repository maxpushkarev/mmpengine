#include <Backend/Dx12/Screen.hpp>

namespace MMPEngine::Backend::Dx12
{
	Screen::Screen(const Settings& settings) : Core::Screen(settings)
	{
		_backBuffer = std::make_shared<BackBuffer>("Screen::BackBuffer");
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<ScreenTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<Screen>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateTaskToUpdate()
	{
		const auto ctx = std::make_shared<ScreenTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<Screen>(shared_from_this());
		return std::make_shared<UpdateTask>(ctx);
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateTaskToSwapBuffer()
	{
		return Core::BaseTask::kEmpty;
	}

	std::shared_ptr<Core::TargetTexture> Screen::GetBackBuffer() const
	{
		return _backBuffer;
	}

	Screen::BackBuffer::BackBuffer(const std::string& name) : Core::TargetTexture(name)
	{
	}

	Screen::InitTask::InitTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task(ctx)
	{
	}

	void Screen::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
	}

	Screen::UpdateTask::UpdateTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task(ctx)
	{
	}

	void Screen::UpdateTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
	}

	Screen::PresentTask::PresentTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task(ctx)
	{
	}

	void Screen::PresentTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
	}

}