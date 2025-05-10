#include <Backend/Vulkan/Compute.hpp>
#include <Core/Material.hpp>
#include <cassert>

namespace MMPEngine::Backend::Vulkan
{
	DirectComputeJob::DirectComputeJob(const std::shared_ptr<Core::ComputeMaterial>& material) : Core::DirectComputeJob(material)
	{
	}

	DirectComputeJob::~DirectComputeJob()
	{
		if (_device && _shaderModule)
		{
			vkDestroyShaderModule(_device->GetNativeLogical(), _shaderModule, nullptr);
		}
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

		job->_device = _specificGlobalContext->device;
		job->PrepareMaterialParameters(_specificGlobalContext, job->_material->GetParameters());

		VkShaderModuleCreateInfo shaderModelInfo{};
		shaderModelInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModelInfo.pNext = nullptr;
		shaderModelInfo.flags = 0;
		shaderModelInfo.codeSize = job->_material->GetShader()->GetCompiledBinaryLength();
		shaderModelInfo.pCode = static_cast<const std::uint32_t*>(job->_material->GetShader()->GetCompiledBinaryData());

		vkCreateShaderModule(_specificGlobalContext->device->GetNativeLogical(), &shaderModelInfo, nullptr, &job->_shaderModule);
		assert(job->_shaderModule);

		VkComputePipelineCreateInfo computePipelineInfo;
		computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineInfo.pNext = nullptr;
		computePipelineInfo.flags = 0;
		computePipelineInfo.layout = job->_pipelineLayout;
		computePipelineInfo.stage = {
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_COMPUTE_BIT,
			job->_shaderModule,
			Core::Shader::ENTRY_POINT_NAME,
			VK_NULL_HANDLE
		};
		computePipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		computePipelineInfo.basePipelineIndex = 0;

		vkCreateComputePipelines(_specificGlobalContext->device->GetNativeLogical(), VK_NULL_HANDLE, 1, &computePipelineInfo, nullptr, &job->_pipeline);
		assert(job->_pipeline);
	}

	DirectComputeJob::ExecutionTask::ExecutionTask(const std::shared_ptr<ExecutionContext>& ctx) : Task(ctx)
	{
		const auto baseJobCtx = std::make_shared<TaskContext>();
		baseJobCtx->job = std::dynamic_pointer_cast<Vulkan::BaseJob>(ctx->job);

		_switchStates = std::make_shared<SwitchState>(baseJobCtx);
		_impl = std::make_shared<Impl>(ctx);
	}

	void DirectComputeJob::ExecutionTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		stream->Schedule(_switchStates);
		stream->Schedule(_impl);
	}

	DirectComputeJob::ExecutionTask::Impl::Impl(const std::shared_ptr<ExecutionContext>& ctx) : Task(ctx)
	{
	}

	void DirectComputeJob::ExecutionTask::Impl::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		const auto tc = GetTaskContext();
		const auto job = tc->job;
		const auto& dim = tc->dimensions;

		vkCmdBindPipeline(
		_specificStreamContext->PopulateCommandsInBuffer()->GetNative(), 
			VK_PIPELINE_BIND_POINT_COMPUTE, 
			job->_pipeline
		);

		vkCmdBindDescriptorSets(
		_specificStreamContext->PopulateCommandsInBuffer()->GetNative(), 
			VK_PIPELINE_BIND_POINT_COMPUTE,
			job->_pipelineLayout, 
			0,
			static_cast<std::uint32_t>(job->_sets.size()), 
			job->_sets.data(), 
			0, 
			nullptr
		);

		vkCmdDispatch(_specificStreamContext->PopulateCommandsInBuffer()->GetNative(), dim.x, dim.y, dim.z);
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
