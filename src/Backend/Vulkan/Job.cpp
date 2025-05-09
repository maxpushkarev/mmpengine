#include <Backend/Vulkan/Job.hpp>

namespace MMPEngine::Backend::Vulkan
{
	BaseJob::BaseJob() = default;
	BaseJob::~BaseJob() = default;

	BaseJob::ApplyParametersTask::ApplyParametersTask(const std::shared_ptr<TaskContext>& context) : Task<TaskContext>(context)
	{
		_switchState = std::make_shared<SwitchState>(context);
		_apply = std::make_shared<Apply>(context);
	}

	void BaseJob::ApplyParametersTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		stream->Schedule(_switchState);
		stream->Schedule(_apply);
	}

	BaseJob::ApplyParametersTask::SwitchState::SwitchState(const std::shared_ptr<TaskContext>& context) : Task<TaskContext>(context)
	{
	}

	void BaseJob::ApplyParametersTask::SwitchState::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		if (const auto job = this->GetTaskContext()->job)
		{
			for (const auto& sst : job->_switchMaterialParametersStateTasks)
			{
				stream->Schedule(sst);
			}
		}
	}

	BaseJob::ApplyParametersTask::Apply::Apply(const std::shared_ptr<TaskContext>& context) : Task<TaskContext>(context)
	{
	}

	void BaseJob::ApplyParametersTask::Apply::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		if (const auto job = this->GetTaskContext()->job; const auto sc = _specificStreamContext)
		{
			for (const auto& applyParameterCallback : job->_applyMaterialParametersCallbacks)
			{
				applyParameterCallback(sc);
			}
		}
	}
}