#include <Core/Task.hpp>

namespace MMPEngine::Core
{
	BaseTask::BaseTask() = default;
	BaseTask::~BaseTask() = default;

	std::shared_ptr<BaseTask> BaseTask::Empty = std::make_shared<BaseTask>();

    void BaseTask::Run(const std::shared_ptr<BaseStream>& stream)
    {
    }
	void BaseTask::Finalize(const std::shared_ptr<BaseStream>& stream)
	{
	}
}