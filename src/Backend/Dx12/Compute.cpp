#include <Backend/Dx12/Compute.hpp>
#include <Core/Material.hpp>
#include <cassert>
#include <Backend/Dx12/d3dx12.h>

namespace MMPEngine::Backend::Dx12
{
	DirectComputeJob::DirectComputeJob(const std::shared_ptr<Core::ComputeMaterial>& material) : Core::DirectComputeJob(material)
	{
	}

	DirectComputeJob::InitTask::InitTask(const std::shared_ptr<InitContext>& ctx) : Task(ctx)
	{
	}

	void DirectComputeJob::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto ac = _specificGlobalContext;
		assert(ac);

		const auto job = GetTaskContext()->job;
		assert(job);

		job->BakeMaterialParameters(_specificGlobalContext, job->_material->GetParameters());

		const auto cs = job->_material->GetShader();
		assert(cs);

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

	DirectComputeJob::ExecutionTask::ExecutionTask(const std::shared_ptr<ExecutionContext>& ctx) : Task(ctx)
	{
		const auto baseJobCtx = std::make_shared<TaskContext>();
		baseJobCtx->job = std::dynamic_pointer_cast<Dx12::BaseJob>(ctx->job);

		_applyMaterial = std::make_shared<ApplyParametersTask>(baseJobCtx);
		_setPipelineState = std::make_shared<SetPipelineStateTask>(baseJobCtx);
		_dispatch = std::make_shared<Dispatch>(ctx);
		_setDescriptorHeaps = std::make_shared<BindDescriptorPoolsTask>(std::make_shared<BindDescriptorPoolsTaskContext>());
	}

	void DirectComputeJob::ExecutionTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		_setDescriptorHeaps->GetTaskContext()->FillDescriptors(_specificGlobalContext);
		stream->Schedule(_setDescriptorHeaps);
		stream->Schedule(_setPipelineState);
		stream->Schedule(_applyMaterial);
		stream->Schedule(_dispatch);
	}

	DirectComputeJob::ExecutionTask::Dispatch::Dispatch(const std::shared_ptr<ExecutionContext>& ctx) : Task(ctx)
	{
	}

	void DirectComputeJob::ExecutionTask::Dispatch::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		const auto& dim = GetTaskContext()->dimensions;
		_specificStreamContext->PopulateCommandsInBuffer()->Dispatch(dim.x, dim.y, dim.z);
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
