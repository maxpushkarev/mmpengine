#include <Core/DrawCall.hpp>

namespace MMPEngine::Core
{
	Camera::DrawCallsJob::DrawCallsJob(const std::shared_ptr<Camera>& camera, std::vector<Item>&& items) : _camera(camera), _items(std::move(items))
	{
	}

	std::shared_ptr<BaseTask> Camera::DrawCallsJob::CreateInitializationTask()
	{
		const auto thisJob = std::dynamic_pointer_cast<DrawCallsJob>(shared_from_this());
		return std::make_shared<FunctionalTask>(
			[thisJob](const auto& stream)
			{
				thisJob->_singleDrawCalls.clear();

				for(const auto& item : thisJob->_items)
				{
					thisJob->_singleDrawCalls.push_back(thisJob->BuildSingleDrawCall(item));
					stream->Schedule(thisJob->_singleDrawCalls.back()->CreateInitializationTask());
				}

				thisJob->_items.clear();
			},
			FunctionalTask::Handler {},
			FunctionalTask::Handler {}
		);
	}

}