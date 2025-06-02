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
		VkRenderPassCreateInfo rpInfo {};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rpInfo.pNext = nullptr;
		rpInfo.flags = 0;

		rpInfo.dependencyCount = 0;
		rpInfo.pDependencies = nullptr;

		for (std::size_t i = 0; i < ctx->colorRenderTargets.size(); ++i)
		{
			const auto& crt = ctx->colorRenderTargets[i];

			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = crt->GetFormat();
			colorAttachment.samples = crt->GetSamplesCount();
			colorAttachment.flags = 0;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			if (ctx->job->_camera->GetTarget().color[i].clear)
			{
				colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			}
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = crt->GetLayout();
			colorAttachment.finalLayout = colorAttachment.initialLayout;

			_attachmentDescriptions.push_back(colorAttachment);
		}

		if (ctx->depthStencil)
		{
			VkAttachmentDescription dsAttachment{};
			dsAttachment.format = ctx->depthStencil->GetFormat();
			dsAttachment.samples = ctx->depthStencil->GetSamplesCount();
			dsAttachment.flags = 0;

			dsAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			if (ctx->job->_camera->GetTarget().depthStencil.clear)
			{
				dsAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			}
			dsAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

			dsAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			dsAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			if (ctx->job->_camera->GetTarget().depthStencil.tex->StencilIncluded())
			{
				if (ctx->job->_camera->GetTarget().depthStencil.clear)
				{
					dsAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				}
				else
				{
					dsAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				}
				dsAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			}

			dsAttachment.initialLayout = ctx->depthStencil->GetLayout();
			dsAttachment.finalLayout = ctx->depthStencil->GetLayout();

			_attachmentDescriptions.push_back(dsAttachment);
		}

		rpInfo.attachmentCount = static_cast<std::uint32_t>(_attachmentDescriptions.size());
		rpInfo.pAttachments = _attachmentDescriptions.data();

		rpInfo.subpassCount = 0;
		rpInfo.pSubpasses = nullptr;

		/*VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = static_cast<std::uint32_t>(_colorAttachmentRefs.size());
		subpass.pColorAttachments = _colorAttachmentRefs.data();*/


		//vkCreateRenderPass(_device->GetNativeLogical(), &rpInfo, nullptr, &_renderPass);

		/*VkFramebufferCreateInfo fbInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = nullptr,
			.renderPass = renderPass,
			.attachmentCount = 2,
			.pAttachments = attachments,
			.width = framebufferWidth,
			.height = framebufferHeight,
			.layers = 1
		};

		VkFramebuffer framebuffer;
		VkResult result = vkCreateFramebuffer(device, &fbInfo, nullptr, &framebuffer);*/
	}

	const std::vector<VkAttachmentDescription>& Camera::DrawCallsJob::Pass::GetAttachmentDescriptions() const
	{
		return _attachmentDescriptions;
	}

	Camera::DrawCallsJob::Pass::~Pass()
	{
		if (_renderPass && _device)
		{
			vkDestroyRenderPass(_device->GetNativeLogical(), _renderPass, nullptr);
		}

		if (_frameBuffer && _device)
		{
			vkDestroyFramebuffer(_device->GetNativeLogical(), _frameBuffer, nullptr);
		}
	}

	const Camera::DrawCallsJob::Pass* Camera::DrawCallsJob::GetOrCreatePass(const std::shared_ptr<InternalTaskContext>& ctx, const std::shared_ptr<Wrapper::Device>& device)
	{

		ctx->attachments.clear();

		for (std::size_t i = 0; i < ctx->colorRenderTargets.size(); ++i)
		{
			const auto c = ctx->colorRenderTargets.at(i);
			ctx->attachments.push_back(c->GetImageView());
		}

		if (ctx->depthStencil)
		{
			ctx->attachments.push_back(ctx->depthStencil->GetImageView());
		}

		const Pass* pass = nullptr;

		for (const auto& p : _cachedPasses)
		{
			const auto& v = std::get<0>(p);

			if (v == ctx->attachments)
			{
				pass = &(std::get<1>(p));
				break;
			}
		}

		if (!pass)
		{
			_cachedPasses.emplace_back(
				ctx->attachments,
				Pass{ctx, device }
			);
			pass = &(std::get<1>(_cachedPasses.back()));
		}

		return pass;
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

		const auto pass = tc->job->GetOrCreatePass(tc, _specificGlobalContext->device);

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
		const auto tc = GetTaskContext();
		//vkCmdEndRenderPass(_specificStreamContext->PopulateCommandsInBuffer()->GetNative());
		const auto pass = tc->job->GetOrCreatePass(tc, _specificGlobalContext->device);

		for (std::size_t i = 0; i < tc->colorRenderTargets.size(); ++i)
		{
			const auto crt = tc->colorRenderTargets.at(i);
			crt->SetLayout(pass->GetAttachmentDescriptions().at(i).finalLayout);
		}

		if (tc->depthStencil)
		{
			tc->depthStencil->SetLayout(pass->GetAttachmentDescriptions().back().finalLayout);
		}
	}

	Camera::DrawCallsJob::Start::Start(const std::shared_ptr<InternalTaskContext>& ctx) : Task(ctx)
	{
		_beginPassTask = std::make_shared<BeginPass>(ctx);

		const auto ds = ctx->job->_camera->GetTarget().depthStencil;

		if (ds.tex)
		{
			VkImageAspectFlags depthAspectBit = VK_IMAGE_ASPECT_DEPTH_BIT;
			VkImageLayout depthLayoutBit = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

			if (ds.tex->StencilIncluded())
			{
				depthAspectBit |= VK_IMAGE_ASPECT_STENCIL_BIT;
				depthLayoutBit = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}

			_memoryBarrierTasks.push_back(std::dynamic_pointer_cast<BaseTexture>(ds.tex->GetUnderlyingTexture())->CreateMemoryBarrierTask(
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				depthLayoutBit,
				VkImageSubresourceRange{
					depthAspectBit, 0, 1, 0, 1
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