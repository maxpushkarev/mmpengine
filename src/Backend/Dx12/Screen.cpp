#include <Backend/Dx12/Screen.hpp>
#include <Core/Texture.hpp>

namespace MMPEngine::Backend::Dx12
{
	Screen::Screen(const Settings& settings) : Core::Screen(settings)
	{
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<ScreenTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<Screen>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateTaskToUpdate()
	{
		const auto ctx = std::make_shared<ScreenTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<Screen>(shared_from_this());

		return std::make_shared<Core::BatchTask>(std::initializer_list<std::shared_ptr<Core::BaseTask>> {
			Core::StreamBarrierTask::kInstance,
			std::make_shared<UpdateTask>(ctx)
		});
	}

	std::shared_ptr<Core::BaseTask> Screen::CreateTaskToSwapBuffer()
	{
		const auto ctx = std::make_shared<ScreenTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<Screen>(shared_from_this());

		return std::make_shared<Core::BatchTask>(std::initializer_list<std::shared_ptr<Core::BaseTask>> {
			Core::StreamFlushTask::kInstance,
			std::make_shared<PresentTask>(ctx)
		});
	}

	std::shared_ptr<Core::ColorTargetTexture> Screen::GetBackBuffer() const
	{
		return _backBuffer;
	}

	Screen::Buffer::Buffer() = default;

	void Screen::Buffer::SetUp(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource, BaseDescriptorPool::Handle&& rtvHandle)
	{
		SetNativeResource(nativeResource, 0);
		_rtvHandle = std::move(rtvHandle);
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

	std::shared_ptr<ResourceEntity> Screen::BackBuffer::GetCurrentBackBuffer() const
	{
		return _buffers[_currentBackBufferIndex];
	}


	Screen::InitTask::InitTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task(ctx)
	{
	}

	void Screen::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto screen = GetTaskContext()->entity;

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
	}

	Screen::UpdateTask::UpdateTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task(ctx)
	{
	}

	void Screen::UpdateTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		const auto screen = GetTaskContext()->entity;

		screen->_swapChain->ResizeBuffers(
			screen->_settings.buffersCount,
			_specificGlobalContext->windowSize.x, _specificGlobalContext->windowSize.y,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

		std::vector<std::shared_ptr<Buffer>> buffers {};
		buffers.resize(screen->_settings.buffersCount);

		for (std::uint32_t i = 0; i < screen->_settings.buffersCount; ++i)
		{
			buffers[i] = std::make_shared<Buffer>();
			Microsoft::WRL::ComPtr<ID3D12Resource> nativeResource {};
			screen->_swapChain->GetBuffer(i, IID_PPV_ARGS(nativeResource.GetAddressOf()));
			
			auto rtvHandle = _specificGlobalContext->rtvDescPool->Allocate();

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
			rtvDesc.Format = screen->_settings.gammaCorrection ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;

			_specificGlobalContext->device->CreateRenderTargetView(nativeResource.Get(), &rtvDesc, rtvHandle.GetCPUDescriptorHandle());
			buffers[i]->SetUp(nativeResource, std::move(rtvHandle));
		}

		screen->_backBuffer = std::make_shared<BackBuffer>(Core::ColorTargetTexture::Settings {
			Core::ColorTargetTexture::Settings::Format::R8G8B8A8_Float_01,
			screen->_settings.clearColor,
			{Core::TargetTexture::Settings::Antialiasing::MSAA_0, _specificGlobalContext->windowSize, "Screen::BackBuffer"}
		}, std::move(buffers));

	}

	Screen::PresentTask::PresentTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task(ctx)
	{
	}

	void Screen::PresentTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto screen = GetTaskContext()->entity;


		screen->_backBuffer->Swap();
	}

}
