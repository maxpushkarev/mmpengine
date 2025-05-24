#include <cassert>
#include <Backend/Vulkan/Screen.hpp>
#include <Core/Texture.hpp>
#include <Backend/Vulkan/Entity.hpp>

#ifdef MMPENGINE_WIN
#include <vulkan/vulkan_win32.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace MMPEngine::Backend::Vulkan
{
	Screen::Screen(const Settings& settings) : Core::Screen(settings)
	{
	}

	Screen::~Screen()
	{
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

	std::shared_ptr<Core::BaseTask> Screen::CreateTaskToSwapBufferInternal()
	{
		const auto ctx = std::make_shared<ScreenTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<Screen>(shared_from_this());

		return std::make_shared<Core::BatchTask>(std::initializer_list<std::shared_ptr<Core::BaseTask>> {
			//ctx->entity->_backBuffer->CreateSwitchStateTask(D3D12_RESOURCE_STATE_PRESENT),
				Core::StreamFlushTask::kInstance,
				std::make_shared<PresentTask>(ctx)
		});
	}

	std::shared_ptr<Core::ColorTargetTexture> Screen::GetBackBuffer() const
	{
		//return _backBuffer;
		return nullptr;
	}

	/*Screen::Buffer::Buffer() = default;

	void Screen::Buffer::SetUp(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource, BaseDescriptorPool::Handle&& rtvHandle, DXGI_FORMAT rtvFormat)
	{
		SetNativeResource(nativeResource, 0);
		_rtvHandle = std::move(rtvHandle);
		_rtvFormat = rtvFormat;
	}

	DXGI_SAMPLE_DESC Screen::Buffer::GetSampleDesc() const
	{
		return { 1, 0 };
	}


	const BaseDescriptorPool::Handle* Screen::Buffer::GetRTVDescriptorHandle() const
	{
		return &_rtvHandle;
	}

	DXGI_FORMAT Screen::Buffer::GetRTVFormat() const
	{
		return _rtvFormat;
	}

	Screen::BackBuffer::BackBuffer(const Settings& settings, std::vector<std::shared_ptr<Buffer>>&& buffers)
		: Core::ColorTargetTexture(settings), _buffers(std::move(buffers))
	{
	}

	void Screen::BackBuffer::Swap()
	{
		++_currentBackBufferIndex;
		_currentBackBufferIndex %= _buffers.size();
	}

	std::shared_ptr<Screen::Buffer> Screen::BackBuffer::GetCurrentBackBuffer() const
	{
		return _buffers[_currentBackBufferIndex];
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> Screen::BackBuffer::GetNativeResource() const
	{
		return GetCurrentBackBuffer()->GetNativeResource();
	}

	D3D12_GPU_VIRTUAL_ADDRESS Screen::BackBuffer::GetNativeGPUAddressWithRequiredOffset() const
	{
		return GetCurrentBackBuffer()->GetNativeGPUAddressWithRequiredOffset();
	}

	const BaseDescriptorPool::Handle* Screen::BackBuffer::GetRTVDescriptorHandle() const
	{
		return GetCurrentBackBuffer()->GetRTVDescriptorHandle();
	}

	DXGI_FORMAT Screen::BackBuffer::GetRTVFormat() const
	{
		return GetCurrentBackBuffer()->GetRTVFormat();
	}

	DXGI_SAMPLE_DESC Screen::BackBuffer::GetSampleDesc() const
	{
		return GetCurrentBackBuffer()->GetSampleDesc();
	}

	std::shared_ptr<Core::BaseTask> Screen::BackBuffer::CreateSwitchStateTask(D3D12_RESOURCE_STATES nextStateMask)
	{
		const auto ctx = std::make_shared<SwitchStateTaskContext>();
		ctx->newStateMask = nextStateMask;
		ctx->entity = std::dynamic_pointer_cast<BackBuffer>(shared_from_this());
		return std::make_shared<SwitchStateTask>(ctx);
	}

	Screen::BackBuffer::SwitchStateTask::SwitchStateTask(const std::shared_ptr<SwitchStateTaskContext>& ctx) : Task(ctx)
	{
		for (const auto& b : ctx->entity->_buffers)
		{
			const auto c = std::make_shared<ResourceEntity::SwitchStateTaskContext>();
			c->entity = b;
			c->nextStateMask = ctx->newStateMask;
			_internalTasks.push_back(std::make_shared<ResourceEntity::SwitchStateTask>(c));
		}
	}

	void Screen::BackBuffer::SwitchStateTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		_internalTasks.at(GetTaskContext()->entity->_currentBackBufferIndex)->Run(stream);
	}*/

	Screen::InitTask::InitTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task(ctx)
	{
	}

	void Screen::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto screen = GetTaskContext()->entity;
		screen->_device = _specificGlobalContext->device;
		screen->_instance = _specificGlobalContext->instance;

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
		//swapChainInfo.surface = 

		//vkCreateSwapchainKHR(_specificGlobalContext->device->GetNativeLogical(), &swapChainInfo, nullptr, &screen->_swapChain);

		/*

		DXGI_SWAP_CHAIN_DESC swapChainDescription;
		swapChainDescription.BufferDesc.Width = _specificGlobalContext->windowSize.x;
		swapChainDescription.BufferDesc.Height = _specificGlobalContext->windowSize.y;
		swapChainDescription.BufferDesc.RefreshRate.Numerator = _specificGlobalContext->screenRefreshRate;
		swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDescription.SampleDesc = { 1,0 };
		swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDescription.BufferCount = screen->_settings.buffersCount;
		swapChainDescription.OutputWindow = _specificGlobalContext->nativeWindow;
		swapChainDescription.Windowed = true;
		swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		_specificGlobalContext->factory->CreateSwapChain(
			_specificStreamContext->GetQueue().Get(),
			&swapChainDescription,
			screen->_swapChain.GetAddressOf());


		screen->_swapChain->ResizeBuffers(
			screen->_settings.buffersCount,
			_specificGlobalContext->windowSize.x, _specificGlobalContext->windowSize.y,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

		std::vector<std::shared_ptr<Buffer>> buffers{};
		buffers.resize(screen->_settings.buffersCount);

		for (std::uint32_t i = 0; i < screen->_settings.buffersCount; ++i)
		{
			buffers[i] = std::make_shared<Buffer>();
			Microsoft::WRL::ComPtr<ID3D12Resource> nativeResource{};
			screen->_swapChain->GetBuffer(i, IID_PPV_ARGS(nativeResource.GetAddressOf()));

			auto rtvHandle = _specificGlobalContext->rtvDescPool->Allocate();

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
			rtvDesc.Format = screen->_settings.gammaCorrection ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;

			_specificGlobalContext->device->CreateRenderTargetView(nativeResource.Get(), &rtvDesc, rtvHandle.GetCPUDescriptorHandle());
			buffers[i]->SetUp(nativeResource, std::move(rtvHandle), rtvDesc.Format);
		}

		screen->_backBuffer = std::make_shared<BackBuffer>(Core::ColorTargetTexture::Settings{
			Core::ColorTargetTexture::Settings::Format::R8G8B8A8_Float_01,
				screen->_settings.clearColor,
			{ Core::TargetTexture::Settings::Antialiasing::MSAA_0, _specificGlobalContext->windowSize, "Screen::BackBuffer" }
			}, std::move(buffers));*/
	}

	Screen::PresentTask::PresentTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task(ctx)
	{
	}

	void Screen::PresentTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto screen = GetTaskContext()->entity;
		//screen->_swapChain->Present(screen->_settings.vSync, 0);
		//screen->_backBuffer->Swap();
		_specificStreamContext->PopulateCommandsInBuffer();
	}

}
