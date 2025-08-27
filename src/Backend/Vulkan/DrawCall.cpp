#include <cassert>
#include <Backend/Vulkan/DrawCall.hpp>

namespace MMPEngine::Backend::Vulkan
{
	Camera::DrawCallsJob::DrawCallsJob(const std::shared_ptr<Core::Camera>& camera, std::vector<Item>&& items)
		: Core::Camera::DrawCallsJob(camera, std::move(items))
	{
	}

	std::vector<std::shared_ptr<Core::BaseTask>>& Camera::DrawCallsJob::GetMemoryBarrierTasks(Core::PassControl<true, IterationImpl>)
	{
		return _memoryBarrierTasks;
	}

	std::shared_ptr<Camera::DrawCallsJob::Pass> Camera::DrawCallsJob::GetPass(Core::PassControl<true, IterationImpl>) const
	{
		return _pass;
	}

	const std::vector<std::shared_ptr<Core::Camera::DrawCallsJob::Iteration>>& Camera::DrawCallsJob::GetIterations(Core::PassControl<true, IterationImpl>) const
	{
		return _iterations;
	}


	Camera::DrawCallsJob::~DrawCallsJob() = default;

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
			if (ctx->job->_camera->GetTarget().color[i].clear && ctx->job->_camera->GetTarget().color[i].tex->GetSettings().clearColor.has_value())
			{
				colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			}
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			_attachmentDescriptions.push_back(colorAttachment);
		}

		if (ctx->depthStencil)
		{
			VkAttachmentDescription dsAttachment{};
			dsAttachment.format = ctx->depthStencil->GetFormat();
			dsAttachment.samples = ctx->depthStencil->GetSamplesCount();
			dsAttachment.flags = 0;

			dsAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			if (ctx->job->_camera->GetTarget().depthStencil.clear && ctx->job->_camera->GetTarget().depthStencil.tex->GetSettings().clearValue.has_value())
			{
				dsAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			}
			dsAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

			dsAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			dsAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			if (ctx->job->_camera->GetTarget().depthStencil.tex->StencilIncluded())
			{
				if (ctx->job->_camera->GetTarget().depthStencil.clear && ctx->job->_camera->GetTarget().depthStencil.tex->GetSettings().clearValue.has_value())
				{
					dsAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				}
				else
				{
					dsAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				}
				dsAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			}

			dsAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			dsAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			_attachmentDescriptions.push_back(dsAttachment);
		}

		rpInfo.attachmentCount = static_cast<std::uint32_t>(_attachmentDescriptions.size());
		rpInfo.pAttachments = _attachmentDescriptions.data();

		std::vector<VkAttachmentReference> colorRefs;

		for (std::size_t i = 0; i < ctx->colorRenderTargets.size(); ++i)
		{
			const auto& crt = ctx->colorRenderTargets[i];
			VkAttachmentReference cRef = {};
			cRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			cRef.attachment = static_cast<std::uint32_t>(i);

			colorRefs.push_back(cRef);
		}

		VkAttachmentReference dsRef{};

		if (ctx->depthStencil)
		{
			dsRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			dsRef.attachment = static_cast<std::uint32_t>(_attachmentDescriptions.size() - 1);
		}

		auto buildVkSubPassDescription = [ctx, this, &colorRefs, &dsRef]() -> auto
		{
			VkSubpassDescription subPass{};

			subPass.inputAttachmentCount = 0;
			subPass.pInputAttachments = nullptr;

			subPass.preserveAttachmentCount = 0;
			subPass.pPreserveAttachments = nullptr;
			subPass.pResolveAttachments = nullptr;

			subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subPass.flags = 0;

			subPass.colorAttachmentCount = static_cast<std::uint32_t>(colorRefs.size());
			subPass.pColorAttachments = colorRefs.data();

			if (ctx->depthStencil)
			{
				subPass.pDepthStencilAttachment = &dsRef;
			}

			return subPass;
		};

		std::vector<VkSubpassDescription> subPassDescriptions {};
		subPassDescriptions.push_back(buildVkSubPassDescription());

		rpInfo.subpassCount = static_cast<std::uint32_t>(subPassDescriptions.size());
		rpInfo.pSubpasses = subPassDescriptions.data();

		const auto rpRes = vkCreateRenderPass(_device->GetNativeLogical(), &rpInfo, nullptr, &_renderPass);
		assert(rpRes == VK_SUCCESS);
	}

	const std::vector<VkAttachmentDescription>& Camera::DrawCallsJob::Pass::GetAttachmentDescriptions() const
	{
		return _attachmentDescriptions;
	}

	VkRenderPass Camera::DrawCallsJob::Pass::GetRenderPass() const
	{
		return _renderPass;
	}

	Camera::DrawCallsJob::Pass::~Pass()
	{
		if (_renderPass && _device)
		{
			vkDestroyRenderPass(_device->GetNativeLogical(), _renderPass, nullptr);
		}
	}

	Camera::DrawCallsJob::FrameBuffer::FrameBuffer(const std::shared_ptr<const InternalTaskContext>& ctx, const std::shared_ptr<Wrapper::Device>& device) : _device(device)
	{
		std::vector<VkImageView> attachmentViews {};

		for (std::size_t i = 0; i < ctx->colorRenderTargets.size(); ++i)
		{
			const auto& crt = ctx->colorRenderTargets[i];
			attachmentViews.push_back(crt->GetImageView());
		}

		if (ctx->depthStencil)
		{
			attachmentViews.push_back(ctx->depthStencil->GetImageView());
		}

		const auto size = ctx->job->_camera->GetTarget().color.front().tex->GetSettings().base.size;

		VkFramebufferCreateInfo fbInfo{};

		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.pNext = nullptr;
		fbInfo.renderPass = ctx->job->_pass->GetRenderPass();
		fbInfo.flags = 0;

		fbInfo.layers = 1;
		fbInfo.width = size.x;
		fbInfo.height = size.y;

		fbInfo.attachmentCount = static_cast<std::uint32_t>(attachmentViews.size());
		fbInfo.pAttachments = attachmentViews.data();

		const auto fbRes = vkCreateFramebuffer(_device->GetNativeLogical(), &fbInfo, nullptr, &_frameBuffer);
		assert(fbRes == VK_SUCCESS);
	}

	Camera::DrawCallsJob::FrameBuffer::~FrameBuffer()
	{
		if (_frameBuffer && _device)
		{
			vkDestroyFramebuffer(_device->GetNativeLogical(), _frameBuffer, nullptr);
		}
	}

	Camera::DrawCallsJob::FrameBuffer::FrameBuffer(FrameBuffer&&) noexcept = default;

	VkFramebuffer Camera::DrawCallsJob::FrameBuffer::GetFrameBuffer() const
	{
		return _frameBuffer;
	}


	const Camera::DrawCallsJob::FrameBuffer* Camera::DrawCallsJob::GetOrCreateFrameBuffer(const std::shared_ptr<InternalTaskContext>& ctx, const std::shared_ptr<Wrapper::Device>& device)
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

		const FrameBuffer* res = nullptr;

		for (const auto& fb : _cachedFrameBuffers)
		{
			const auto& v = std::get<0>(fb);

			if (v == ctx->attachments)
			{
				res = &(std::get<1>(fb));
				break;
			}
		}

		if (!res)
		{
			_cachedFrameBuffers.emplace_back(
				ctx->attachments,
				FrameBuffer{ctx, device }
			);
			res = &(std::get<1>(_cachedFrameBuffers.back()));
		}

		return res;
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
		ctx->clearValues.clear();

		for (const auto& crt : ctx->job->_camera->GetTarget().color)
		{
			ctx->colorRenderTargets.push_back(std::dynamic_pointer_cast<IColorTargetTexture>(crt.tex->GetUnderlyingTexture()));

			ctx->clearValues.push_back(VkClearValue{});
			const auto clearColor = crt.tex->GetSettings().clearColor;
			if (crt.clear && clearColor.has_value())
			{
				std::memcpy(&ctx->clearValues.back().color, &(clearColor.value()), sizeof(clearColor.value()));
			}
		}

		if (ds.tex)
		{
			ctx->depthStencil = std::dynamic_pointer_cast<IDepthStencilTexture>(ds.tex->GetUnderlyingTexture());

			ctx->clearValues.push_back(VkClearValue {});
			const auto clearValue = ds.tex->GetSettings().clearValue;
			if (ds.clear && clearValue.has_value())
			{
				ctx->clearValues.back().depthStencil = {std::get<0>(clearValue.value()), std::get<1>(clearValue.value()) };
			}
		}

		return ctx;
	}


	std::shared_ptr<Camera::DrawCallsJob::Iteration> Camera::DrawCallsJob::BuildIteration(const Item& item) const
	{
		if (std::dynamic_pointer_cast<Core::MeshMaterial>(item.material))
		{
			return std::make_shared<IterationJob<Core::MeshMaterial>>(std::const_pointer_cast<DrawCallsJob>(std::dynamic_pointer_cast<const DrawCallsJob>(shared_from_this())), item);
		}

		return nullptr;
	}

	std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::CreateInitializationTaskInternal()
	{
		return std::make_shared<InitInternalTask>(BuildInternalContext());
	}

	std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::CreateTaskForIterationsStart()
	{
		return std::make_shared<StartTask>(BuildInternalContext());
	}

	std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::CreateTaskForIterationsFinish()
	{
		return std::make_shared<EndPass>(BuildInternalContext());
	}

	Camera::DrawCallsJob::InitInternalTask::InitInternalTask(const std::shared_ptr<InternalTaskContext>& ctx) : Task<MMPEngine::Backend::Vulkan::Camera::DrawCallsJob::InternalTaskContext>(ctx)
	{
	}

	void Camera::DrawCallsJob::InitInternalTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto ctx = GetTaskContext();

		ctx->job->_pass = std::make_shared<Pass>(ctx, _specificGlobalContext->device);

		if (ctx->job->_camera->GetTarget().depthStencil.tex)
		{
			VkImageAspectFlags depthAspectBit = VK_IMAGE_ASPECT_DEPTH_BIT;
			VkImageLayout depthLayoutBit = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			if (ctx->job->_camera->GetTarget().depthStencil.tex->StencilIncluded())
			{
				depthAspectBit |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}

			ctx->job->_memoryBarrierTasks.push_back(std::dynamic_pointer_cast<BaseTexture>(ctx->job->_camera->GetTarget().depthStencil.tex->GetUnderlyingTexture())->CreateMemoryBarrierTask(
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VkImageSubresourceRange{
					depthAspectBit, 0, 1, 0, 1
				}
			));
		}

		for (const auto& crt : ctx->job->_camera->GetTarget().color)
		{
			ctx->job->_memoryBarrierTasks.push_back(std::dynamic_pointer_cast<BaseTexture>(crt.tex->GetUnderlyingTexture())->CreateMemoryBarrierTask(
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VkImageSubresourceRange{
					VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
				}
			));
		}
	}


	Camera::DrawCallsJob::StartTask::StartTask(const std::shared_ptr<InternalTaskContext>& ctx) : Task<MMPEngine::Backend::Vulkan::Camera::DrawCallsJob::InternalTaskContext>(ctx)
	{
		const auto baseJobCtx = std::make_shared<TaskContext>();
		baseJobCtx->job = ctx->job;
		_memBarriersTasks = std::make_shared<MemBarriersTask>(baseJobCtx);
		_beginPass = std::make_shared<BeginPass>(ctx);
	}

	void Camera::DrawCallsJob::StartTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		stream->Schedule(_memBarriersTasks);
		stream->Schedule(_beginPass);
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

		const auto pass = tc->job->_pass;
		const auto fb = tc->job->GetOrCreateFrameBuffer(tc, _specificGlobalContext->device);
		const auto size = tc->job->_camera->GetTarget().color.front().tex->GetSettings().base.size;

		VkRenderPassBeginInfo rpBeginInfo {};

		rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpBeginInfo.pNext = nullptr;

		rpBeginInfo.framebuffer = fb->GetFrameBuffer();
		rpBeginInfo.renderPass = pass->GetRenderPass();

		rpBeginInfo.renderArea.offset = { 0, 0 };
		rpBeginInfo.renderArea.extent = { size.x, size.y };

		rpBeginInfo.clearValueCount = static_cast<std::uint32_t>(tc->clearValues.size());
		rpBeginInfo.pClearValues = tc->clearValues.data();

		vkCmdBeginRenderPass(_specificStreamContext->PopulateCommandsInBuffer()->GetNative(), &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	Camera::DrawCallsJob::EndPass::EndPass(const std::shared_ptr<InternalTaskContext>& ctx) : Task<MMPEngine::Backend::Vulkan::Camera::DrawCallsJob::InternalTaskContext>(ctx)
	{
	}

	void Camera::DrawCallsJob::EndPass::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		const auto tc = GetTaskContext();

		vkCmdEndRenderPass(_specificStreamContext->PopulateCommandsInBuffer()->GetNative());

		const auto pass = tc->job->_pass;

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

	Camera::DrawCallsJob::IterationImpl::IterationImpl(const std::shared_ptr<DrawCallsJob>& job, const Item& item) : _item(item), _drawCallsJob(job.get())
	{
	}
}