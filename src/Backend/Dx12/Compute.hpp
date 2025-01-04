#pragma once
#include <Core/Compute.hpp>
#include <Backend/Dx12/Task.hpp>
#include <Backend/Dx12/Job.hpp>

namespace MMPEngine::Backend::Dx12
{
	class DirectComputeJob final : public Core::DirectComputeJob, public Dx12::Job<Core::ComputeMaterial>
	{
	private:
		class InitContext final : public Core::TaskContext
		{
		public:
			std::shared_ptr<DirectComputeJob> job;
		};

		class ExecutionContext final : public Core::DirectComputeContext
		{
		public:
			std::shared_ptr<DirectComputeJob> job;
		};

		class InitTask final : public Task<InitContext>
		{
		public:
			InitTask(const std::shared_ptr<InitContext>& ctx);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class ExecutionTask final : public Task<Core::DirectComputeContext>
		{
		private:
			class SetPipelineState final : public Task<ExecutionContext>
			{
			public:
				SetPipelineState(const std::shared_ptr<ExecutionContext>& ctx);
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

			class Dispatch final : public Task<ExecutionContext>
			{
			public:
				Dispatch(const std::shared_ptr<ExecutionContext>& ctx);
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};


		public:
			ExecutionTask(const std::shared_ptr<ExecutionContext>& ctx);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		private:
			std::shared_ptr<ExecutionContext> _executionContext;
			std::shared_ptr<BindDescriptorPoolsTask> _setDescriptorHeaps;
			std::shared_ptr<BaseTask> _applyMaterial;
			std::shared_ptr<BaseTask> _setPipelineState;
			std::shared_ptr<Dispatch> _dispatch;
 		};

	public:
		DirectComputeJob(const std::shared_ptr<Core::ComputeMaterial>& material);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::ContextualTask<Core::DirectComputeContext>> CreateExecutionTask() override;
	};
}
