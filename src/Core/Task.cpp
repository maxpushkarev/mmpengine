#include <Core/Task.hpp>

namespace MMPEngine::Core
{
	BaseTask::BaseTask() = default;
	BaseTask::~BaseTask() = default;

	std::shared_ptr<BaseTask> BaseTask::kEmpty = std::make_shared<BaseTask>();

    void BaseTask::Run(const std::shared_ptr<BaseStream>& stream)
    {
    }
	void BaseTask::Finalize(const std::shared_ptr<BaseStream>& stream)
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
}