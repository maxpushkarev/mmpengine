#include <Backend/Vulkan/Texture.hpp>
#include <cassert>

namespace MMPEngine::Backend::Vulkan
{
	ITargetTexture::ITargetTexture() = default;
	ITargetTexture::~ITargetTexture() = default;

	std::shared_ptr<Core::BaseTask> BaseTexture::CreateMemoryBarrierTask(
		VkAccessFlags srcAccess, 
		VkAccessFlags dstAccess, 
		VkImageLayout newLayout, 
		const VkImageSubresourceRange& subResourceRange,
		VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
	{
		const auto ctx = std::make_shared<MemoryBarrierContext>();
		ctx->entity = std::dynamic_pointer_cast<BaseTexture>(shared_from_this());
		ctx->data.srcAccess = srcAccess;
		ctx->data.dstAccess = dstAccess;
		ctx->data.srcStage = srcStage;
		ctx->data.dstStage = dstStage;
		ctx->data.imageLayout = newLayout;
		ctx->data.subresourceRange = subResourceRange;

		return std::make_shared<MemoryBarrierTask>(ctx);
	}

	BaseTexture::MemoryBarrierTask::MemoryBarrierTask(const std::shared_ptr<MemoryBarrierContext>& ctx) : Task<MMPEngine::Backend::Vulkan::BaseTexture::MemoryBarrierContext>(ctx)
	{
	}

	VkSampleCountFlagBits BaseTexture::GetSampleCount(Core::TargetTexture::Settings::Antialiasing aa)
	{
		switch (aa)
		{
		case Core::TargetTexture::Settings::Antialiasing::MSAA_0:
			return VK_SAMPLE_COUNT_1_BIT;
		case Core::TargetTexture::Settings::Antialiasing::MSAA_2:
			return VK_SAMPLE_COUNT_2_BIT;
		case Core::TargetTexture::Settings::Antialiasing::MSAA_4:
			return VK_SAMPLE_COUNT_4_BIT;
		case Core::TargetTexture::Settings::Antialiasing::MSAA_8:
			return VK_SAMPLE_COUNT_8_BIT;
		}

		throw Core::UnsupportedException("MSAA type unknown");
	}


	void BaseTexture::MemoryBarrierTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto ctx = GetTaskContext();
		const auto entity = ctx->entity;

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;

		barrier.oldLayout = entity->_layout;
		barrier.newLayout = ctx->data.imageLayout;

		barrier.srcAccessMask = ctx->data.srcAccess;
		barrier.dstAccessMask = ctx->data.dstAccess;

		barrier.image = entity->_nativeImage;
		barrier.subresourceRange = ctx->data.subresourceRange;

		barrier.srcQueueFamilyIndex = ctx->entity->_queueFamilyIndexOwnerShip.value_or(_specificStreamContext->GetQueue()->GetFamilyIndex());
		barrier.dstQueueFamilyIndex = _specificStreamContext->GetQueue()->GetFamilyIndex();

		vkCmdPipelineBarrier(
			_specificStreamContext->PopulateCommandsInBuffer()->GetNative(),
			ctx->data.srcStage,
			ctx->data.dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		ctx->entity->_queueFamilyIndexOwnerShip = _specificStreamContext->GetQueue()->GetFamilyIndex();
		ctx->entity->_layout = ctx->data.imageLayout;
	}

	ResourceTexture::ResourceTexture() = default;
	ResourceTexture::~ResourceTexture()
	{
		if (_view && _device)
		{
			vkDestroyImageView(_device->GetNativeLogical(), _view, nullptr);
		}

		if (_nativeImage && _device)
		{
			vkDestroyImage(_device->GetNativeLogical(), _nativeImage, nullptr);
		}
	}

	std::shared_ptr<DeviceMemoryHeap> ResourceTexture::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
	{
		return globalContext->residentBufferHeap;
	}


	ResourceTexture::BindTask::BindTask(const std::shared_ptr<TaskContext>& context) : Task<MMPEngine::Backend::Vulkan::ResourceTexture::TaskContext>(context)
	{
	}


	void ResourceTexture::BindTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto tc = GetTaskContext();

		const auto res = vkBindImageMemory(
			tc->entity->_device->GetNativeLogical(),
			tc->entity->_nativeImage,
			tc->entity->_deviceMemoryHeapHandle.GetMemoryBlock()->GetNative(),
			static_cast<VkDeviceSize>(tc->entity->_deviceMemoryHeapHandle.GetOffset())
		);

		assert(res == VK_SUCCESS);
	}



	DepthStencilTargetTexture::DepthStencilTargetTexture(const Settings& settings) : Core::DepthStencilTargetTexture(settings)
	{
	}

	VkFormat DepthStencilTargetTexture::GetFormat() const
	{
		const auto& settings = GetSettings();

		switch (settings.format)
		{
		case Settings::Format::Depth16:
			return VK_FORMAT_D16_UNORM;
		case Settings::Format::Depth32:
			return VK_FORMAT_D32_SFLOAT;
		case Settings::Format::Depth24_Stencil8:
			return VK_FORMAT_D24_UNORM_S8_UINT;
		}

		throw Core::UnsupportedException("unknown depth/stencil format");
	}


	std::shared_ptr<Core::BaseTask> DepthStencilTargetTexture::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<DepthStencilTargetTexture>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	DepthStencilTargetTexture::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task(ctx)
	{
	}

	void DepthStencilTargetTexture::InitTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		stream->Schedule(std::make_shared<CreateImage>(GetTaskContext()));
		stream->Schedule(GetTaskContext()->entity->GetMemoryHeap(_specificGlobalContext)->CreateTaskToInitializeBlocks());

		const auto resTexTaskContext = std::make_shared<ResourceTexture::TaskContext>();
		resTexTaskContext->entity = GetTaskContext()->entity;
		stream->Schedule(std::make_shared<BindTask>(resTexTaskContext));
		stream->Schedule(std::make_shared<CreateView>(GetTaskContext()));
	}

	DepthStencilTargetTexture::InitTask::CreateImage::CreateImage(const std::shared_ptr<InitTaskContext>& context)
		: Task<MMPEngine::Backend::Vulkan::DepthStencilTargetTexture::InitTaskContext>(context)
	{
	}

	void DepthStencilTargetTexture::InitTask::CreateImage::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		const auto dsTex = GetTaskContext()->entity;
		dsTex->_device = _specificGlobalContext->device;

		const auto& settings = dsTex->GetSettings();
		const auto& size = settings.base.size;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = nullptr;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = size.x;
		imageInfo.extent.height = size.y;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = dsTex->_layout;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.pQueueFamilyIndices = nullptr;
		imageInfo.queueFamilyIndexCount = 0;
		imageInfo.format = dsTex->GetFormat();
		imageInfo.samples = GetSampleCount(settings.base.antialiasing);
		imageInfo.flags = 0;

		const auto createDsImage = vkCreateImage(dsTex->_device->GetNativeLogical(), &imageInfo, nullptr, &dsTex->_nativeImage);
		assert(createDsImage == VK_SUCCESS);

		const auto memHeap = dsTex->GetMemoryHeap(_specificGlobalContext);
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(dsTex->_device->GetNativeLogical(), dsTex->_nativeImage, &memRequirements);

		dsTex->_deviceMemoryHeapHandle = memHeap->Allocate(Core::Heap::Request{
			static_cast<std::size_t>(memRequirements.size),
			static_cast<std::size_t>(memRequirements.alignment)
		});
	}

	DepthStencilTargetTexture::InitTask::CreateView::CreateView(const std::shared_ptr<InitTaskContext>& context)
		: Task<MMPEngine::Backend::Vulkan::DepthStencilTargetTexture::InitTaskContext>(context)
	{
	}

	void DepthStencilTargetTexture::InitTask::CreateView::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto dsTex = GetTaskContext()->entity;

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.flags = 0;
		viewInfo.image = dsTex->_nativeImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = dsTex->GetFormat();
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (dsTex->StencilIncluded())
		{
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		const auto imgViewRes = vkCreateImageView(dsTex->_device->GetNativeLogical(), &viewInfo, nullptr, &dsTex->_view);
		assert(imgViewRes == VK_SUCCESS);
	}

}