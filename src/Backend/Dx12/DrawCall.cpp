#include <Backend/Dx12/DrawCall.hpp>

namespace MMPEngine::Backend::Dx12
{
	Camera::DrawCallsJob::DrawCallsJob(const std::shared_ptr<Core::Camera>& camera, std::vector<Item>&& items)
		: Core::Camera::DrawCallsJob(camera, std::move(items))
	{
	}


	std::shared_ptr<Camera::DrawCallsJob::Iteration> Camera::DrawCallsJob::BuildIteration(const Item& item) const
	{
		return nullptr;
	}

	std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::CreateTaskForIterationsStart()
	{
		const auto ctx = std::make_shared<InternalTaskContext>();
		ctx->job = std::dynamic_pointer_cast<DrawCallsJob>(shared_from_this());

		const auto ds = ctx->job->_camera->GetTarget().depthStencil;
		if(ds.tex)
		{
			ctx->depthStencilDescriptor = std::dynamic_pointer_cast<BaseEntity>(ds.tex->GetUnderlyingTexture())->GetShaderVisibleDescriptorHandle()->GetCPUDescriptorHandle();
		}

		for(const auto& crt : ctx->job->_camera->GetTarget().color)
		{
			ctx->colorRenderTargetDescriptors.push_back(std::dynamic_pointer_cast<BaseEntity>(crt.tex->GetUnderlyingTexture())->GetShaderVisibleDescriptorHandle()->GetCPUDescriptorHandle());
		}

		return std::make_shared<PrepareTask>(ctx);
	}

	Camera::DrawCallsJob::PrepareRenderTargetsTask::PrepareRenderTargetsTask(const std::shared_ptr<InternalTaskContext>& ctx) : Task(ctx)
	{
	}

	void Camera::DrawCallsJob::PrepareRenderTargetsTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto tc= GetTaskContext();
		const auto ds = tc->job->_camera->GetTarget().depthStencil;
		const auto crts = tc->job->_camera->GetTarget().color;

		if(ds.tex && ds.clear)
		{
			const auto& clearValue = ds.tex->GetSettings().clearValue;
			if(clearValue.has_value())
			{
				_specificStreamContext->PopulateCommandsInList()->ClearDepthStencilView(
					tc->depthStencilDescriptor,
					D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
					std::get<std::float_t>(clearValue.value()), std::get<std::uint8_t>(clearValue.value()),
					0, nullptr
				);
			}
		}

		for(std::size_t i = 0; i < crts.size(); ++i)
		{
			const auto& crt = crts[i];
			if (crt.clear)
			{
				const auto& clearValue = crt.tex->GetSettings().clearColor;
				if(clearValue.has_value())
				{
					const auto cv  = clearValue.value();
					_specificStreamContext->PopulateCommandsInList()->ClearRenderTargetView(tc->colorRenderTargetDescriptors.at(i), &(cv.x), 0, nullptr);
				}
			}
		}
	}

	Camera::DrawCallsJob::PrepareTask::PrepareTask(const std::shared_ptr<InternalTaskContext>& ctx) : Task(ctx)
	{
		_prepareRenderTargets = std::make_shared<PrepareRenderTargetsTask>(ctx);
		_bindDescriptorHeaps = std::make_shared<BindDescriptorPoolsTask>(std::make_shared<BindDescriptorPoolsTaskContext>());

		const auto ds = ctx->job->_camera->GetTarget().depthStencil;

		if(ds.tex)
		{
			_switchStateTasks.push_back(std::dynamic_pointer_cast<BaseEntity>(ds.tex->GetUnderlyingTexture())->CreateSwitchStateTask(D3D12_RESOURCE_STATE_DEPTH_WRITE));
		}

		for (const auto& crt : ctx->job->_camera->GetTarget().color)
		{
			_switchStateTasks.push_back(std::dynamic_pointer_cast<BaseEntity>(crt.tex->GetUnderlyingTexture())->CreateSwitchStateTask(D3D12_RESOURCE_STATE_RENDER_TARGET));
		}
	}

	void Camera::DrawCallsJob::PrepareTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		_bindDescriptorHeaps->GetTaskContext()->FillDescriptors(_specificGlobalContext);

		for(const auto& sst : _switchStateTasks)
		{
			stream->Schedule(sst);
		}

		stream->Schedule(_bindDescriptorHeaps);
		stream->Schedule(_prepareRenderTargets);
	}

}