#include <Backend/Dx12/Compute.hpp>
#include <Backend/Dx12/Material.hpp>
#include <cassert>

namespace MMPEngine::Backend::Dx12
{
	DirectComputeJob::DirectComputeJob(const std::shared_ptr<Core::ComputeMaterial>& material) : Core::DirectComputeJob(material)
	{
	}

	DirectComputeJob::InitTask::InitTask(const std::shared_ptr<InitContext>& ctx) : ContextualTask(ctx)
	{
	}

	void DirectComputeJob::InitTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
	}

	void DirectComputeJob::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto job = _internalTaskContext->job.lock();
		assert(job);
		const auto cs = job->_material->GetShader();
		assert(cs);

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};

		psoDesc.CS =
		{
			cs->GetCompiledBinaryData(),
			cs->GetCompiledBinaryLength()
		};
		psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	}

	void DirectComputeJob::InitTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

	DirectComputeJob::ExecutionTask::ExecutionTask(const std::shared_ptr<ExecutionContext>& ctx) : ContextualTask(ctx), _executionContext(ctx)
	{
		_applyMaterial = ctx->job.lock()->_material->CreateTaskForApply();
	}

	void DirectComputeJob::ExecutionTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
		if(const auto job = _executionContext->job.lock())
		{
			stream->Schedule(_applyMaterial);
		}
	}

	void DirectComputeJob::ExecutionTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
	}

	void DirectComputeJob::ExecutionTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

	std::shared_ptr<Core::BaseTask> DirectComputeJob::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitContext>();
		ctx->job = std::dynamic_pointer_cast<DirectComputeJob>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	std::shared_ptr<Core::ContextualTask<Core::DirectComputeContext>> DirectComputeJob::CreateExecutionTask()
	{
		const auto ctx = std::make_shared<ExecutionContext>();
		ctx->job = std::dynamic_pointer_cast<DirectComputeJob>(shared_from_this());
		return std::make_shared<ExecutionTask>(ctx);
	}

}
