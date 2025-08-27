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
				},
				FunctionalTask::Handler{},
				FunctionalTask::Handler{}
			)
		});
	}

	std::shared_ptr<BaseTask> Camera::DrawCallsJob::CreateExecutionTask()
	{
        const auto ctx = std::make_shared<ExecutionTaskContext>();
        ctx->job = std::dynamic_pointer_cast<DrawCallsJob>(shared_from_this());
        return std::make_shared<ExecutionTask>(ctx);
	}

    Camera::DrawCallsJob::ExecutionTask::ExecutionTask(const std::shared_ptr<ExecutionTaskContext>& ctx) : ContextualTask<ExecutionTaskContext>(ctx)
    {
        _iterationsStart = ctx->job->CreateTaskForIterationsStart();
        _iterationsFinish = ctx->job->CreateTaskForIterationsFinish();
        
        _iterationExecutionTasks.reserve(ctx->job->_iterations.size());
        std::transform(ctx->job->_iterations.cbegin(), ctx->job->_iterations.cend(), std::back_inserter(_iterationExecutionTasks), [](const auto& i)
        {
            return i->CreateExecutionTask();
        });
    }
    
    void Camera::DrawCallsJob::ExecutionTask::OnScheduled(const std::shared_ptr<BaseStream>& stream)
    {
        ContextualTask::OnScheduled(stream);
        
        stream->Schedule(_iterationsStart);
        
        const auto job = GetTaskContext()->job;
        
        for(std::size_t i = 0; i < job->_items.size(); ++i)
        {
            if(job->_items.at(i).renderer->IsActive())
            {
                stream->Schedule(_iterationExecutionTasks.at(i));
            }
        }
        
        stream->Schedule(_iterationsFinish);
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
