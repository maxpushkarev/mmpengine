#pragma once
#include <Core/Job.hpp>
#include <Core/Camera.hpp>
#include <Core/Geometry.hpp>

namespace MMPEngine::Core
{
	class Camera::DrawCallsJob : public Job<void>
	{
	protected:
		class Iteration : public Job<void>
		{
		};
	public:
		struct Item final
		{
			std::shared_ptr<IGeometryRenderer> renderer;
			std::shared_ptr<BaseMaterial> material;
		};
		DrawCallsJob(const std::shared_ptr<Camera>& camera, std::vector<Item>&& items);
		std::shared_ptr<BaseTask> CreateInitializationTask() override;
		std::shared_ptr<BaseTask> CreateExecutionTask() override;
    private:
        class ExecutionTaskContext : public TaskContext
        {
        public:
            std::shared_ptr<DrawCallsJob> job;
        };
        
        class ExecutionTask final : public ContextualTask<ExecutionTaskContext>
        {
        public:
            ExecutionTask(const std::shared_ptr<ExecutionTaskContext>& ctx);
        protected:
            void OnScheduled(const std::shared_ptr<BaseStream>& stream) override;
        private:
            std::shared_ptr<BaseTask> _iterationsStart;
            std::shared_ptr<BaseTask> _iterationsFinish;
            std::vector<std::shared_ptr<BaseTask>> _iterationExecutionTasks;
        };
        
	protected:
		virtual std::shared_ptr<BaseTask> CreateInitializationTaskInternal();
		virtual std::shared_ptr<BaseTask> CreateTaskForIterationsStart() = 0;
		virtual std::shared_ptr<Iteration> BuildIteration(const Item& item) const = 0;
		virtual std::shared_ptr<BaseTask> CreateTaskForIterationsFinish();
		std::shared_ptr<Camera> _camera;
		std::vector<Item> _items;
		std::vector<std::shared_ptr<Iteration>> _iterations;
	};
}
