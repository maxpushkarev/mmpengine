#include <Core/Task.hpp>

namespace MMPEngine::Core
{
	BaseTask::BaseTask() = default;
	BaseTask::~BaseTask() = default;

	std::shared_ptr<BaseTask> BaseTask::kEmpty = std::make_shared<BaseTask>();

    void BaseTask::Run(const std::shared_ptr<BaseStream>& stream)
    {
    }

	void BaseTask::OnScheduled(const std::shared_ptr<BaseStream>& stream)
	{
	}

	void BaseTask::OnComplete(const std::shared_ptr<BaseStream>& stream)
	{
	}

	void StreamBarrierTask::Run(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::Run(stream);

		stream->SubmitInternal();
		stream->SyncInternal();
		stream->RestartInternal();
	}

	std::shared_ptr<StreamBarrierTask> StreamBarrierTask::kInstance = std::make_shared<StreamBarrierTask>();

	void StreamFlushTask::Run(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::Run(stream);
		stream->Flush();
	}

	std::shared_ptr<StreamFlushTask> StreamFlushTask::kInstance = std::make_shared<StreamFlushTask>();

	void BatchTask::OnScheduled(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::OnScheduled(stream);

		for(const auto& t : _tasks)
		{
			stream->Schedule(t);
		}
	}

	BatchTask::BatchTask(std::initializer_list<std::shared_ptr<BaseTask>> tasks) : _tasks(tasks)
	{
	}

	BatchTask::BatchTask(std::vector<std::shared_ptr<BaseTask>>&& tasks) : _tasks(std::move(tasks))
	{
	}

	FunctionalTask::FunctionalTask(Handler&& onScheduleFn, Handler&& runFn, Handler&& onCompleteFn)
		: _onScheduleFn(std::move(onScheduleFn)), _runFn(std::move(runFn)), _onOnCompleteFn(std::move(onCompleteFn))
	{
	}

	void FunctionalTask::OnScheduled(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::OnScheduled(stream);

		if(_onScheduleFn)
		{
			_onScheduleFn(stream);
		}
	}

	void FunctionalTask::Run(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::Run(stream);

		if(_runFn)
		{
			_runFn(stream);
		}
	}

	void FunctionalTask::OnComplete(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::OnComplete(stream);

		if(_onOnCompleteFn)
		{
			_onOnCompleteFn(stream);
		}
	}

}