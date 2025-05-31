#include <cassert>
#include <Backend/Vulkan/Screen.hpp>
#include <Core/Texture.hpp>
#include <Backend/Vulkan/Entity.hpp>
#include <Backend/Vulkan/Texture.hpp>

#ifdef MMPENGINE_WIN
#include <vulkan/vulkan_win32.h>
#endif

namespace MMPEngine::Backend::Vulkan
{
	Screen::Screen(const Settings& settings) : Core::Screen(settings)
	{
	}

	std::size_t Screen::GetCurrentImageIndex() const
	{
		return _backBuffer->GetCurrentImageIndex();
	}

	Screen::~Screen()
	{
		if (_semaphore && _device)
		{
			vkDestroySemaphore(_device->GetNativeLogical(), _semaphore, nullptr);	
		}

		if (_swapChain && _device)
		{
			vkDestroySwapchainKHR(_device->GetNativeLogical(), _swapChain, nullptr);
		}

		if (_surface && _instance)
		{
			vkDestroySurfaceKHR(_instance->GetNative(), _surface, nullptr);
		}
	}


	std::shared_ptr<Core::BaseTask> Screen::CreateInitializationTaskInternal()
	{
		const auto ctx = std::make_shared<ScreenTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<Screen>(shared_from_this());

		return std::make_shared<InitTask>(ctx);
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateStartFrameTaskInternal()
	{
		const auto ctx = std::make_shared<ScreenTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<Screen>(shared_from_this());

		return std::make_shared<StartFrameTask>(ctx);
	}

	std::shared_ptr<Core::BaseTask> Screen::CreatePresentationTaskInternal()
	{
		const auto ctx = std::make_shared<ScreenTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<Screen>(shared_from_this());

		return std::make_shared<Core::BatchTask>(std::initializer_list<std::shared_ptr<Core::BaseTask>> {
				ctx->entity->_backBuffer->CreateMemoryBarrierTask(
					VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT,
					0,
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					VkImageSubresourceRange{
						VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
					},
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
				),
				Core::StreamFlushTask::kInstance,
				std::make_shared<PresentTask>(ctx)
		});
	}

	std::shared_ptr<Core::ColorTargetTexture> Screen::GetBackBuffer() const
	{
		return _backBuffer;
	}


	Screen::Image::Image(VkImage image)
	{
		_nativeImage = image;
	}

	std::shared_ptr<DeviceMemoryHeap> Screen::Image::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
	{
		return nullptr;
	}

	Screen::Image::~Image() = default;


	Screen::BackBuffer::BackBuffer(const Settings& settings, VkSwapchainKHR swapChain, const std::shared_ptr<Wrapper::Device>& device)
		: Core::ColorTargetTexture(settings)
	{
		std::uint32_t imageCount {};
		vkGetSwapchainImagesKHR(device->GetNativeLogical(), swapChain, &imageCount, nullptr);
		std::vector<VkImage> swapChainImages(imageCount);
		vkGetSwapchainImagesKHR(device->GetNativeLogical(), swapChain, &imageCount, swapChainImages.data());

		_images.reserve(imageCount);
		for (const auto img : swapChainImages)
		{
			_images.push_back(std::make_shared<Image>(img));
		}
	}

	void Screen::BackBuffer::Swap()
	{
		++_currentImageIndex;
		_currentImageIndex %= _images.size();
	}

	std::shared_ptr<Screen::Image> Screen::BackBuffer::GetCurrentImage() const
	{
		return _images[_currentImageIndex];
	}

	std::size_t Screen::BackBuffer::GetCurrentImageIndex() const
	{
		return _currentImageIndex;
	}


	std::shared_ptr<DeviceMemoryHeap> Screen::BackBuffer::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
	{
		return nullptr;
	}

	std::shared_ptr<Core::BaseTask> Screen::BackBuffer::CreateMemoryBarrierTask(VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkImageLayout newLayout, const VkImageSubresourceRange& subResourceRange, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
	{
		const auto ctx = std::make_shared<MemoryBarrierContextInternal>();
		ctx->entity = std::dynamic_pointer_cast<BackBuffer>(shared_from_this());

		ctx->data.srcAccess = srcAccess;
		ctx->data.dstAccess = dstAccess;
		ctx->data.srcStage = srcStage;
		ctx->data.dstStage = dstStage;
		ctx->data.imageLayout = newLayout;
		ctx->data.subresourceRange = subResourceRange;

		return std::make_shared<MemoryBarrierTaskInternal>(ctx);
	}

	Screen::BackBuffer::MemoryBarrierTaskInternal::MemoryBarrierTaskInternal(const std::shared_ptr<MemoryBarrierContextInternal>& ctx) : Task<MMPEngine::Backend::Vulkan::Screen::BackBuffer::MemoryBarrierContextInternal>(ctx)
	{
		for (const auto& i : ctx->entity->_images)
		{
			const auto c = std::make_shared<MemoryBarrierContext>();
			c->entity = i;
			c->data = ctx->data;
			_internalTasks.push_back(std::make_shared<MemoryBarrierTask>(c));
		}
	}

	void Screen::BackBuffer::MemoryBarrierTaskInternal::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		_internalTasks.at(GetTaskContext()->entity->_currentImageIndex)->Run(stream);
	}

	Screen::InitTask::InitTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task(ctx)
	{
	}

	void Screen::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto screen = GetTaskContext()->entity;
		screen->_device = _specificGlobalContext->device;
		screen->_instance = _specificGlobalContext->instance;

		VkSemaphoreCreateInfo semInfo {};
		semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semInfo.pNext = nullptr;
		semInfo.flags = 0;
		vkCreateSemaphore(_specificGlobalContext->device->GetNativeLogical(), &semInfo, nullptr, &screen->_semaphore);

#ifdef MMPENGINE_WIN
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = _specificGlobalContext->nativeWindow;
		createInfo.hinstance = GetModuleHandle(nullptr);
		createInfo.pNext = nullptr;
		createInfo.flags = 0;

		const auto surfaceCreationRes= vkCreateWin32SurfaceKHR(_specificGlobalContext->instance->GetNative(), &createInfo, nullptr, &screen->_surface);
		assert(surfaceCreationRes == VK_SUCCESS);
#endif

		VkSwapchainCreateInfoKHR swapChainInfo = {};
		swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainInfo.pNext = nullptr;
		swapChainInfo.flags = 0;
		swapChainInfo.clipped = VK_FALSE;
		swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainInfo.imageArrayLayers = 1;
		swapChainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapChainInfo.imageFormat = screen->_settings.gammaCorrection ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
		swapChainInfo.imageExtent = {_specificGlobalContext->windowSize.x, _specificGlobalContext->windowSize.y };
		swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapChainInfo.oldSwapchain = VK_NULL_HANDLE;
		swapChainInfo.minImageCount = screen->_settings.buffersCount;
		swapChainInfo.queueFamilyIndexCount = 0;
		swapChainInfo.pQueueFamilyIndices = nullptr;
		swapChainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		swapChainInfo.surface = screen->_surface;
		swapChainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

		const auto swapChainRes = vkCreateSwapchainKHR(_specificGlobalContext->device->GetNativeLogical(), &swapChainInfo, nullptr, &screen->_swapChain);
		assert(swapChainRes == VK_SUCCESS);

		screen->_backBuffer = std::make_shared<BackBuffer>(Core::ColorTargetTexture::Settings{
			Core::ColorTargetTexture::Settings::Format::R8G8B8A8_Float_01,
				screen->_settings.clearColor,
			{ Core::TargetTexture::Settings::Antialiasing::MSAA_0, _specificGlobalContext->windowSize, "Screen::BackBuffer" }
			}, screen->_swapChain, _specificGlobalContext->device);
	}

	Screen::StartFrameTask::StartFrameTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task<MMPEngine::Backend::Vulkan::Screen::ScreenTaskContext>(ctx)
	{
	}

	void Screen::StartFrameTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto screen = GetTaskContext()->entity;
		std::uint64_t waitTime = (std::numeric_limits<std::uint64_t>::max)();
		vkAcquireNextImageKHR(
			_specificGlobalContext->device->GetNativeLogical(), 
			screen->_swapChain, 
			waitTime, 
			screen->_semaphore, 
			VK_NULL_HANDLE, 
			&screen->_acquireNextImageIndex
		);
	}


	Screen::PresentTask::PresentTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task(ctx)
	{
	}

	void Screen::PresentTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		
		const auto screen = GetTaskContext()->entity;
		const auto currentImageIndex = static_cast<std::uint32_t>(screen->GetCurrentImageIndex());

		VkPresentInfoKHR presentInfo {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.pResults = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &screen->_swapChain;
		presentInfo.pWaitSemaphores = &screen->_semaphore;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pImageIndices = &currentImageIndex;
		vkQueuePresentKHR(_specificStreamContext->GetQueue()->GetNative(), &presentInfo);
		
		screen->_backBuffer->Swap();
	}

}
