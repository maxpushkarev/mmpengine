#include <Backend/Dx12/Compute.hpp>
#include <Backend/Dx12/Material.hpp>
#include <cassert>
#include <Backend/Dx12/d3dx12.h>

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

		const auto ac = _specificAppContext;
		assert(ac);

		const auto job = _taskContext->job;
		assert(job);
		const auto cs = job->_material->GetShader();
		assert(cs);

		const auto material = std::dynamic_pointer_cast<Dx12::ComputeMaterial>(job->_material->GetUnderlyingMaterial());
		assert(material);

		job->_rootSignature = material->GetRootSignature();
		assert(job->_rootSignature);

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};

		psoDesc.CS =
		{
			cs->GetCompiledBinaryData(),
			cs->GetCompiledBinaryLength()
		};

		psoDesc.pRootSignature = job->_rootSignature.Get();
		psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		auto psoStream = CD3DX12_PIPELINE_STATE_STREAM{ psoDesc };
		D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
		streamDesc.pPipelineStateSubobjectStream = &psoStream;
		streamDesc.SizeInBytes = sizeof(psoStream);

		Microsoft::WRL::ComPtr<ID3D12Device2> castedDevice = nullptr;
		ac->device->QueryInterface(IID_PPV_ARGS(castedDevice.GetAddressOf()));
		assert(castedDevice != nullptr);

		castedDevice->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&job->_pipelineState));
	}

	void DirectComputeJob::InitTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

	DirectComputeJob::ExecutionTask::ExecutionTask(const std::shared_ptr<ExecutionContext>& ctx) : ContextualTask(ctx), _executionContext(ctx)
	{
		_applyMaterial = ctx->job->_material->CreateTaskForApply();
		_setPipelineState = std::make_shared<SetPipelineState>(ctx);
		_dispatch = std::make_shared<Dispatch>(ctx);
		_setDescriptorHeaps = std::make_shared<BindDescriptorHeapsTask>(std::make_shared<BindDescriptorHeapsTaskContext>());
	}

	void DirectComputeJob::ExecutionTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		_setDescriptorHeaps->GetContext()->FillDescriptors(_specificAppContext);
		stream->Schedule(_setDescriptorHeaps);
		stream->Schedule(_setPipelineState);
		stream->Schedule(_applyMaterial);
		stream->Schedule(_dispatch);
	}

	void DirectComputeJob::ExecutionTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
	}

	void DirectComputeJob::ExecutionTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

	DirectComputeJob::ExecutionTask::SetPipelineState::SetPipelineState(const std::shared_ptr<ExecutionContext>& ctx) : ContextualTask(ctx)
	{
	}

	void DirectComputeJob::ExecutionTask::SetPipelineState::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
	}

	void DirectComputeJob::ExecutionTask::SetPipelineState::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		if (const auto job = _taskContext->job)
		{
			_specificStreamContext->PopulateCommandsInList()->SetPipelineState(job->_pipelineState.Get());
			_specificStreamContext->PopulateCommandsInList()->SetComputeRootSignature(job->_rootSignature.Get());
		}
	}

	void DirectComputeJob::ExecutionTask::SetPipelineState::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

	DirectComputeJob::ExecutionTask::Dispatch::Dispatch(const std::shared_ptr<ExecutionContext>& ctx) : ContextualTask(ctx)
	{
	}

	void DirectComputeJob::ExecutionTask::Dispatch::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
	}

	void DirectComputeJob::ExecutionTask::Dispatch::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		const auto& dim = _taskContext->dimensions;
		_specificStreamContext->PopulateCommandsInList()->Dispatch(dim.x, dim.y, dim.z);
	}

	void DirectComputeJob::ExecutionTask::Dispatch::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
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
