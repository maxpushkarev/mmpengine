#include <cassert>
#include <Core/DrawCall.hpp>
#include <Core/Mesh.hpp>

namespace MMPEngine::Core
{
	Camera::DrawCallsJob::DrawCallsJob(const std::shared_ptr<Camera>& camera, std::vector<Item>&& items) : _camera(camera), _items(std::move(items))
	{
	}

	std::shared_ptr<BaseTask> Camera::DrawCallsJob::CreateInitializationTask()
	{
		const auto thisJob = std::dynamic_pointer_cast<DrawCallsJob>(shared_from_this());

		return std::make_shared<BatchTask>(std::initializer_list<std::shared_ptr<BaseTask>>{
			CreateInitializationTaskInternal(),
			std::make_shared<FunctionalTask>(
				[thisJob](const auto& stream)
				{
					thisJob->_iterations.clear();

					for (const auto& item : thisJob->_items)
					{
						if (const auto mr = std::dynamic_pointer_cast<Mesh::Renderer>(item.renderer))
						{
							assert(std::dynamic_pointer_cast<Core::MeshMaterial>(item.material) != nullptr);
						}

						thisJob->_iterations.push_back(thisJob->BuildIteration(item));
						stream->Schedule(thisJob->_iterations.back()->CreateInitializationTask());
					}

					thisJob->_items.clear();
				},
				FunctionalTask::Handler{},
				FunctionalTask::Handler{}
			)
		});
	}

	std::shared_ptr<BaseTask> Camera::DrawCallsJob::CreateExecutionTask()
	{
		std::vector<std::shared_ptr<BaseTask>> iterationExecutionTasks {};
		iterationExecutionTasks.reserve(_iterations.size());
		std::transform(_iterations.cbegin(), _iterations.cend(), std::back_inserter(iterationExecutionTasks), [](const auto& i)
		{
			return i->CreateExecutionTask();
		});

		return std::make_shared<BatchTask>(std::initializer_list<std::shared_ptr<BaseTask>>{
			CreateTaskForIterationsStart(),
			std::make_shared<BatchTask>(std::move(iterationExecutionTasks)),
			CreateTaskForIterationsFinish()
		});
	}

	std::shared_ptr<BaseTask> Camera::DrawCallsJob::CreateTaskForIterationsFinish()
	{
		return BaseTask::kEmpty;
	}

	std::shared_ptr<BaseTask> Camera::DrawCallsJob::CreateInitializationTaskInternal()
	{
		return BaseTask::kEmpty;
	}
}