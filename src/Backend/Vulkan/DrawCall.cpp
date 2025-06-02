#include <cassert>
#include <Backend/Vulkan/DrawCall.hpp>

namespace MMPEngine::Backend::Vulkan
{
	Camera::DrawCallsJob::DrawCallsJob(const std::shared_ptr<Core::Camera>& camera, std::vector<Item>&& items)
		: Core::Camera::DrawCallsJob(camera, std::move(items))
	{
	}

	Camera::DrawCallsJob::~DrawCallsJob() = default;

	Camera::DrawCallsJob::Pass::Pass(Pass&&) noexcept = default;

	Camera::DrawCallsJob::Pass::Pass(const std::shared_ptr<const InternalTaskContext>& ctx, const std::shared_ptr<Wrapper::Device>& device) : _device(device)
	{
	}

	Camera::DrawCallsJob::Pass::~Pass()
	{
		if (_frameBuffer && _device)
		{
			vkDestroyFramebuffer(_device->GetNativeLogical(), _frameBuffer, nullptr);
		}
	}


	std::shared_ptr<Camera::DrawCallsJob::InternalTaskContext> Camera::DrawCallsJob::BuildInternalContext()
	{
		const auto ctx = std::make_shared<InternalTaskContext>();
		ctx->job = std::dynamic_pointer_cast<DrawCallsJob>(shared_from_this());

		ctx->colorRenderTargets.clear();
		ctx->colorRenderTargets.reserve(ctx->job->_camera->GetTarget().color.size());

		ctx->attachments.clear();
		ctx->attachments.resize(ctx->job->_camera->GetTarget().color.size() + 1);

		const auto ds = ctx->job->_camera->GetTarget().depthStencil;
		if (ds.tex)
		{
			ctx->depthStencil = std::dynamic_pointer_cast<IDepthStencilTexture>(ds.tex->GetUnderlyingTexture());
		}

		for (const auto& crt : ctx->job->_camera->GetTarget().color)
		{
			ctx->colorRenderTargets.push_back(std::dynamic_pointer_cast<IColorTargetTexture>(crt.tex->GetUnderlyingTexture()));
		}

		return ctx;
	}


	std::shared_ptr<Camera::DrawCallsJob::Iteration> Camera::DrawCallsJob::BuildIteration(const Item& item) const
	{
		/*if (std::dynamic_pointer_cast<Core::MeshMaterial>(item.material))
		{
			return std::make_shared<IterationJob<Core::MeshMaterial>>(_camera, item);
		}*/

		return nullptr;
	}

	std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::CreateTaskForIterationsStart()
	{
		return std::make_shared<Start>(BuildInternalContext());
	}

	std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::CreateTaskForIterationsFinish()
	{
		return std::make_shared<EndPass>(BuildInternalContext());
	}

	Camera::DrawCallsJob::BeginPass::BeginPass(const std::shared_ptr<InternalTaskContext>& ctx) : Task(ctx)
	{
	}

	void Camera::DrawCallsJob::BeginPass::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto tc = GetTaskContext();
		const auto ds = tc->job->_camera->GetTarget().depthStencil;
		const auto crts = tc->job->_camera->GetTarget().color;

		tc->attachments.clear();

		for (std::size_t i = 0; i < crts.size(); ++i)
		{
			const auto& crt = crts[i];
			const auto c = tc->colorRenderTargets.at(i);
			tc->attachments.push_back(c->GetImageView());
		}

		if (ds.tex)
		{
			tc->attachments.push_back(tc->depthStencil->GetImageView());
		}

		const Pass* pass = nullptr;

		for (const auto& p : tc->job->_cachedPasses)
		{
			const auto& views = std::get<0>(p);

			if (views == tc->attachments)
			{
				pass = &(std::get<1>(p));
				break;
			}
		}

		if (!pass)
		{
			tc->job->_cachedPasses.emplace_back(
				tc->attachments,
				Pass{ tc, _specificGlobalContext->device }
			);
			pass = &(std::get<1>(tc->job->_cachedPasses.back()));
		}


		/*const D3D12_CPU_DESCRIPTOR_HANDLE* dsHandlePtr = nullptr;

		if (ds.tex)
		{
			dsHandlePtr = &(tc->depthStencil->GetDSVDescriptorHandle()->GetCPUDescriptorHandle());
		}

		if (ds.tex && ds.clear)
		{
			const auto& clearValue = ds.tex->GetSettings().clearValue;
			if (clearValue.has_value())
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

		for (std::size_t i = 0; i < crts.size(); ++i)
		{
			const auto& crt = crts[i];
			const auto h = tc->colorRenderTargets.at(i)->GetRTVDescriptorHandle()->GetCPUDescriptorHandle();

			tc->colorRenderTargetHandles.push_back(h);

			if (crt.clear)
			{
				const auto& clearValue = crt.tex->GetSettings().clearColor;

				if (clearValue.has_value())
				{
					const auto cv = clearValue.value();
					_specificStreamContext->PopulateCommandsInBuffer()->ClearRenderTargetView(h, &(cv.x), 0, nullptr);
				}
			}
		}

		D3D12_VIEWPORT viewport{};
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
			dsHandlePtr);*/
	}

	Camera::DrawCallsJob::EndPass::EndPass(const std::shared_ptr<InternalTaskContext>& ctx) : Task<MMPEngine::Backend::Vulkan::Camera::DrawCallsJob::InternalTaskContext>(ctx)
	{
	}

	void Camera::DrawCallsJob::EndPass::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		//vkCmdEndRenderPass(_specificStreamContext->PopulateCommandsInBuffer()->GetNative());
	}

	Camera::DrawCallsJob::Start::Start(const std::shared_ptr<InternalTaskContext>& ctx) : Task(ctx)
	{
		_beginPassTask = std::make_shared<BeginPass>(ctx);

		const auto ds = ctx->job->_camera->GetTarget().depthStencil;

		if (ds.tex)
		{
			VkImageAspectFlags aspectBit = VK_IMAGE_ASPECT_DEPTH_BIT;
			VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

			if (ds.tex->StencilIncluded())
			{
				aspectBit |= VK_IMAGE_ASPECT_STENCIL_BIT;
				layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}

			_memoryBarrierTasks.push_back(std::dynamic_pointer_cast<BaseTexture>(ds.tex->GetUnderlyingTexture())->CreateMemoryBarrierTask(
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				layout,
				VkImageSubresourceRange{
					aspectBit, 0, 1, 0, 1
				}
			));
		}

		for (const auto& crt : ctx->job->_camera->GetTarget().color)
		{
			_memoryBarrierTasks.push_back(std::dynamic_pointer_cast<BaseTexture>(crt.tex->GetUnderlyingTexture())->CreateMemoryBarrierTask(
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VkImageSubresourceRange{
					VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
				}
			));
		}
	}

	void Camera::DrawCallsJob::Start::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		for (const auto& sst : _memoryBarrierTasks)
		{
			stream->Schedule(sst);
		}

		stream->Schedule(_beginPassTask);
	}
}