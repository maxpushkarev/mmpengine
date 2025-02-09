#include <cassert>
#include <Backend/Dx12/DrawCall.hpp>

namespace MMPEngine::Backend::Dx12
{
	Camera::DrawCallsJob::DrawCallsJob(const std::shared_ptr<Core::Camera>& camera, std::vector<Item>&& items)
		: Core::Camera::DrawCallsJob(camera, std::move(items))
	{
	}


	std::shared_ptr<Camera::DrawCallsJob::Iteration> Camera::DrawCallsJob::BuildIteration(const Item& item) const
	{
		if(std::dynamic_pointer_cast<Core::MeshMaterial>(item.material))
		{
			return std::make_shared<IterationJob<Core::MeshMaterial>>(_camera, item);
		}

		return nullptr;
	}

	std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::CreateTaskForIterationsStart()
	{
		const auto ctx = std::make_shared<InternalTaskContext>();
		ctx->job = std::dynamic_pointer_cast<DrawCallsJob>(shared_from_this());

		ctx->colorRenderTargets.clear();
		ctx->colorRenderTargetHandles.clear();

		ctx->colorRenderTargets.reserve(ctx->job->_camera->GetTarget().color.size());
		ctx->colorRenderTargetHandles.reserve(ctx->job->_camera->GetTarget().color.size());

		const auto ds = ctx->job->_camera->GetTarget().depthStencil;
		if(ds.tex)
		{
			ctx->depthStencil = std::dynamic_pointer_cast<BaseEntity>(ds.tex->GetUnderlyingTexture());
		}

		for(const auto& crt : ctx->job->_camera->GetTarget().color)
		{
			ctx->colorRenderTargets.push_back(std::dynamic_pointer_cast<BaseEntity>(crt.tex->GetUnderlyingTexture()));
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

		const D3D12_CPU_DESCRIPTOR_HANDLE* dsHandlePtr = nullptr;

		if(ds.tex)
		{
			dsHandlePtr = &(tc->depthStencil->GetDSVDescriptorHandle()->GetCPUDescriptorHandle());
		}

		if(ds.tex && ds.clear)
		{
			const auto& clearValue = ds.tex->GetSettings().clearValue;
			if(clearValue.has_value())
			{
				_specificStreamContext->PopulateCommandsInBuffer()->ClearDepthStencilView(
					*dsHandlePtr,
					D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
					std::get<std::float_t>(clearValue.value()), std::get<std::uint8_t>(clearValue.value()),
					0, nullptr
				);
			}
		}

		tc->colorRenderTargetHandles.clear();

		for(std::size_t i = 0; i < crts.size(); ++i)
		{
			const auto& crt = crts[i];
			const auto h = tc->colorRenderTargets.at(i)->GetRTVDescriptorHandle()->GetCPUDescriptorHandle();

			tc->colorRenderTargetHandles.push_back(h);

			if (crt.clear)
			{
				const auto& clearValue = crt.tex->GetSettings().clearColor;

				if(clearValue.has_value())
				{
					const auto cv  = clearValue.value();
					_specificStreamContext->PopulateCommandsInBuffer()->ClearRenderTargetView(h, &(cv.x), 0, nullptr);
				}
			}
		}

		D3D12_VIEWPORT viewport {};
		const auto size = crts.begin()->tex->GetSettings().base.size;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<std::float_t>(size.x);
		viewport.Height = static_cast<std::float_t>(size.y);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		const D3D12_RECT scissorsRect = { 0, 0, static_cast<decltype(scissorsRect.right)>(size.x), static_cast<decltype(scissorsRect.bottom)>(size.y) };

		_specificStreamContext->PopulateCommandsInBuffer()->RSSetViewports(1, &viewport);
		_specificStreamContext->PopulateCommandsInBuffer()->RSSetScissorRects(1, &scissorsRect);

		_specificStreamContext->PopulateCommandsInBuffer()->OMSetRenderTargets(
			static_cast<std::uint32_t>(tc->colorRenderTargetHandles.size()),
			tc->colorRenderTargetHandles.data(),
			false,
			dsHandlePtr);
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