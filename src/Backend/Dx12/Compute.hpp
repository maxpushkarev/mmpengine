#pragma once
#include <Core/Compute.hpp>

#include "Task.hpp"

namespace MMPEngine::Backend::Dx12
{
	class DirectComputeJob final : public Core::DirectComputeJob
	{
	private:

		class InitContext final : public Core::TaskContext
		{
		public:
			std::weak_ptr<DirectComputeJob> _job;
		};

		class ExecutionContext final : public Core::DirectComputeContext
		{
		public:
			std::weak_ptr<DirectComputeJob> _job;
		};

		class InitTask final : public Task, public Core::TaskWithContext<InitContext>
		{
		public:
			InitTask(const std::shared_ptr<InitContext>& ctx);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			void OnComplete(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class ExecutionTask final : public Task, public Core::TaskWithContext<Core::DirectComputeContext>
		{
		public:
			ExecutionTask(const std::shared_ptr<ExecutionContext>& ctx);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			void OnComplete(const std::shared_ptr<Core::BaseStream>& stream) override;
		private:
			std::shared_ptr<ExecutionContext> _executionContext;
		};

	public:
		DirectComputeJob(const std::shared_ptr<Core::ComputeMaterial>& material);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::TaskWithContext<Core::DirectComputeContext>> CreateExecutionTask() override;
	};
}
