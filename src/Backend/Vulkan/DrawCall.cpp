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

		std::vector<VkImageView> attachmentViews;

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
			colorAttachment.initialLayout = crt->GetLayout();
			colorAttachment.finalLayout = colorAttachment.initialLayout;

			_attachmentDescriptions.push_back(colorAttachment);
			attachmentViews.push_back(crt->GetImageView());
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

			dsAttachment.initialLayout = ctx->depthStencil->GetLayout();
			dsAttachment.finalLayout = ctx->depthStencil->GetLayout();

			_attachmentDescriptions.push_back(dsAttachment);
			attachmentViews.push_back(ctx->depthStencil->GetImageView());
		}

		rpInfo.attachmentCount = static_cast<std::uint32_t>(_attachmentDescriptions.size());
		rpInfo.pAttachments = _attachmentDescriptions.data();

		std::vector<VkAttachmentReference> colorRefs;

		for (std::size_t i = 0; i < ctx->colorRenderTargets.size(); ++i)
		{
			const auto& crt = ctx->colorRenderTargets[i];
			VkAttachmentReference cRef = {};
			cRef.layout = crt->GetLayout();
			cRef.attachment = static_cast<std::uint32_t>(i);

			colorRefs.push_back(cRef);
		}

		VkAttachmentReference dsRef{};

		if (ctx->depthStencil)
		{
			dsRef.layout = ctx->depthStencil->GetLayout();
			dsRef.attachment = static_cast<std::uint32_t>(_attachmentDescriptions.size() - 1);
		}

		auto buildVkSubPassSubscription = [ctx, this, &colorRefs, &dsRef]() -> auto
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
		subPassDescriptions.push_back(buildVkSubPassSubscription());

		for (std::size_t i = 0; i < ctx->job->_iterations.size(); ++i)
		{
			subPassDescriptions.push_back(buildVkSubPassSubscription());
		}

		rpInfo.subpassCount = static_cast<std::uint32_t>(subPassDescriptions.size());
		rpInfo.pSubpasses = subPassDescriptions.data();

		const auto rpRes = vkCreateRenderPass(_device->GetNativeLogical(), &rpInfo, nullptr, &_renderPass);
		assert(rpRes == VK_SUCCESS);

		const auto size = ctx->job->_camera->GetTarget().color.front().tex->GetSettings().base.size;

		VkFramebufferCreateInfo fbInfo {};

		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.pNext = nullptr;
		fbInfo.renderPass = _renderPass;
		fbInfo.flags = 0;

		fbInfo.layers = 1;
		fbInfo.width = size.x;
		fbInfo.height = size.y;

		fbInfo.attachmentCount = static_cast<std::uint32_t>(attachmentViews.size());
		fbInfo.pAttachments = attachmentViews.data();

		const auto fbRes = vkCreateFramebuffer(_device->GetNativeLogical(), &fbInfo, nullptr, &_frameBuffer);
		assert(fbRes == VK_SUCCESS);
	}

	const std::vector<VkAttachmentDescription>& Camera::DrawCallsJob::Pass::GetAttachmentDescriptions() const
	{
		return _attachmentDescriptions;
	}

	VkFramebuffer Camera::DrawCallsJob::Pass::GetFrameBuffer() const
	{
		return _frameBuffer;
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
			return std::make_shared<IterationJob<Core::MeshMaterial>>(_camera, item);
		}

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
		const auto size = tc->job->_camera->GetTarget().color.front().tex->GetSettings().base.size;

		VkRenderPassBeginInfo rpBeginInfo {};

		rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpBeginInfo.pNext = nullptr;

		rpBeginInfo.framebuffer = pass->GetFrameBuffer();
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
			VkImageLayout depthLayoutBit = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			if (ds.tex->StencilIncluded())
			{
				depthAspectBit |= VK_IMAGE_ASPECT_STENCIL_BIT;
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