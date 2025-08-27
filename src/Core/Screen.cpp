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

	std::shared_ptr<Screen> Screen::GetUnderlyingScreen()
	{
		return std::dynamic_pointer_cast<Screen>(shared_from_this());
	}

	std::shared_ptr<BaseTask> Screen::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<TaskContext>();
		ctx->screen = std::dynamic_pointer_cast<Screen>(shared_from_this());

		return std::make_shared<StaticBatchTask>(std::initializer_list<std::shared_ptr<BaseTask>>{
			std::make_shared<StreamValidationTask>(ctx),
			CreateInitializationTaskInternal()
		});
	}

	std::shared_ptr<BaseTask> Screen::CreatePresentationTask()
	{
		const auto ctx = std::make_shared<TaskContext>();
		ctx->screen = std::dynamic_pointer_cast<Screen>(shared_from_this());

		return std::make_shared<StaticBatchTask>(std::initializer_list<std::shared_ptr<BaseTask>>{
			std::make_shared<StreamValidationTask>(ctx),
				std::make_shared<FunctionalTask>(
					[](const auto&)
					{
					},
					[ctx](const auto&)
					{
						if (!ctx->screen->_readyForPresentation)
						{
							throw std::runtime_error("screen not ready for presentation");
						}
					},
					[](const auto&) {}
				),
				ctx->screen->GetUnderlyingScreen()->CreatePresentationTaskInternal(),
				std::make_shared<FunctionalTask>(
					[](const auto&)
					{
					},
					[ctx](const auto&)
					{
						ctx->screen->_readyForPresentation = false;
					},
					[](const auto&) {}
				),
		});
	}

	std::shared_ptr<BaseTask> Screen::CreateStartFrameTask()
	{
		const auto ctx = std::make_shared<TaskContext>();
		ctx->screen = std::dynamic_pointer_cast<Screen>(shared_from_this());

		return std::make_shared<StaticBatchTask>(std::initializer_list<std::shared_ptr<BaseTask>>{
			std::make_shared<StreamValidationTask>(ctx),
			std::make_shared<FunctionalTask>(
			[](const auto&)
			{
			},
				[ctx](const auto&)
				{
					ctx->screen->_readyForPresentation = true;
				},
				[](const auto&) {}
			),
			ctx->screen->GetUnderlyingScreen()->CreateStartFrameTaskInternal()
		});
	}

	std::shared_ptr<ContextualTask<Screen::FrameTaskContext>> Screen::CreateFrameTask()
	{
		const auto frameTaskContext = std::make_shared<FrameTaskContextImpl>();
		frameTaskContext->screen = std::dynamic_pointer_cast<Screen>(shared_from_this());
		return std::make_shared<FrameTask>(frameTaskContext);
	}

	Screen::FrameTask::FrameTask(const std::shared_ptr<FrameTaskContextImpl>& ctx) : ContextualTask<MMPEngine::Core::Screen::FrameTaskContext>(ctx)
	{
		_startFrame = ctx->screen->CreateStartFrameTask();
		_endFrame = ctx->screen->CreatePresentationTask();
	}

	void Screen::FrameTask::OnScheduled(const std::shared_ptr<BaseStream>& stream)
	{
		ContextualTask::OnScheduled(stream);

		stream->Schedule(_startFrame);
		if (const auto internalFrameTask = GetTaskContext()->internalTask)
		{
			stream->Schedule(internalFrameTask);
		}
		stream->Schedule(_endFrame);
	}

}