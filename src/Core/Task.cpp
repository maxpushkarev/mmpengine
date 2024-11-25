#include <Core/Task.hpp>

namespace MMPEngine::Core
{
	BaseTask::BaseTask() = default;
	BaseTask::~BaseTask() = default;

    void BaseTask::Run(const std::shared_ptr<BaseStream>& stream)
    {
    }
	void BaseTask::Finalize(const std::shared_ptr<BaseStream>& stream)
	{
	}
}