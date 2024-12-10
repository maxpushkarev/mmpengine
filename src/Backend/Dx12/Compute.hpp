#pragma once
#include <Core/Compute.hpp>
#include <Backend/Dx12/Task.hpp>

namespace MMPEngine::Backend::Dx12
{
	class DirectComputeJob final : public Core::DirectComputeJob
	{
	private:

		class InitContext final : public Core::TaskContext
		{
		public:
			std::weak_ptr<DirectComputeJob> job;
		};

		class ExecutionContext final : public Core::DirectComputeContext
		{
		public:
			std::weak_ptr<DirectComputeJob> job;
		};

		class InitTask final : public Task, public Core::ContextualTask<InitContext>
		{
		public:
			InitTask(const std::shared_ptr<InitContext>& ctx);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			void OnComplete(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class ExecutionTask final : public Task, public Core::ContextualTask<Core::DirectComputeContext>
		{
		private:
			class SetPipelineState final : public Task, public Core::ContextualTask<ExecutionContext>
			{
			public:
				SetPipelineState(const std::shared_ptr<ExecutionContext>& ctx);
				void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
				void OnComplete(const std::shared_ptr<Core::BaseStream>& stream) override;
			};
		public:
			ExecutionTask(const std::shared_ptr<ExecutionContext>& ctx);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			void OnComplete(const std::shared_ptr<Core::BaseStream>& stream) override;
		private:
			std::shared_ptr<ExecutionContext> _executionContext;
			std::shared_ptr<BaseTask> _applyMaterial;
			std::shared_ptr<SetPipelineState> _setPipelineState;
 		};

		Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;

	public:
		DirectComputeJob(const std::shared_ptr<Core::ComputeMaterial>& material);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::ContextualTask<Core::DirectComputeContext>> CreateExecutionTask() override;
	};
}
