#include <Backend/Dx12/Compute.hpp>
#include <Backend/Dx12/Material.hpp>
#include <cassert>

namespace MMPEngine::Backend::Dx12
{
	DirectComputeJob::DirectComputeJob(const std::shared_ptr<Core::ComputeMaterial>& material) : Core::DirectComputeJob(material)
	{
	}

	DirectComputeJob::InitTask::InitTask(const std::shared_ptr<InitContext>& ctx) : TaskWithContext(ctx)
	{
	}

	void DirectComputeJob::InitTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
	}

	void DirectComputeJob::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
	}

	void DirectComputeJob::InitTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

	DirectComputeJob::ExecutionTask::ExecutionTask(const std::shared_ptr<ExecutionContext>& ctx) : TaskWithContext(ctx), _executionContext(ctx)
	{
	}

	void DirectComputeJob::ExecutionTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
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
		ctx->_job = std::dynamic_pointer_cast<DirectComputeJob>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	std::shared_ptr<Core::TaskWithContext<Core::DirectComputeContext>> DirectComputeJob::CreateExecutionTask()
	{
		const auto ctx = std::make_shared<ExecutionContext>();
		ctx->_job = std::dynamic_pointer_cast<DirectComputeJob>(shared_from_this());
		return std::make_shared<ExecutionTask>(ctx);
	}

}
