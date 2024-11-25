#include <algorithm>
#include <Backend/Dx12/App.hpp>
#include <array>
#include <cassert>
#include <Backend/Dx12/Stream.hpp>

namespace MMPEngine::Backend::Dx12
{
	RootApp::RootApp(const std::shared_ptr<AppContext>& context)
		: Core::RootApp<AppContext>(context)
	{
	}

	void RootApp::Initialize()
	{
		if(_rootContext->settings.isDebug)
		{
			Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
			D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
			debugController->EnableDebugLayer();
		}

		CreateDXGIFactory(IID_PPV_ARGS(&_rootContext->factory));


		constexpr std::array<D3D_FEATURE_LEVEL, 3> featureLevels = { D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0 };

		for (const auto featureLevel : featureLevels)
		{
			std::int32_t i = 0;
			IDXGIAdapter* adapter = nullptr;

			while (_rootContext->factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
			{
				DXGI_ADAPTER_DESC desc{};
				adapter->GetDesc(&desc);

				D3D12CreateDevice(adapter, featureLevel, IID_PPV_ARGS(&_rootContext->device));
				++i;

				if (_rootContext->device != nullptr)
				{
					std::wstring wAdapterName = { desc.Description };
					std::string adapterName(wAdapterName.length(), 0);
					std::transform(wAdapterName.cbegin(), wAdapterName.cend(), adapterName.begin(), [](wchar_t c) {
						return static_cast<char>(c);
						});
					_rootContext->logger->Log(Core::ELogType::Info, "D3D12 device created. adapter: ", adapterName);
					break;
				}
			}

			if (_rootContext->device != nullptr)
			{
				break;
			}
		}

		assert(_rootContext->device != nullptr);


		_rootContext->rtvHeap = std::make_shared<RTVDescriptorHeap>(_rootContext->device, Core::BaseItemHeap::Settings{});
		_rootContext->dsvHeap = std::make_shared<DSVDescriptorHeap>(_rootContext->device, Core::BaseItemHeap::Settings{});
		_rootContext->cbvSrvUavShaderInVisibleHeap = std::make_shared<CBVSRVUAVDescriptorHeap>(_rootContext->device, Core::BaseItemHeap::Settings{}, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		_rootContext->cbvSrvUavShaderVisibleHeap = std::make_shared<CBVSRVUAVDescriptorHeap>(_rootContext->device, Core::BaseItemHeap::Settings{}, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

		const auto streamContext = std::make_shared<StreamContext>();

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		_rootContext->device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(streamContext->cmdQueue.GetAddressOf()));

		_rootContext->device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&streamContext->fence));

		_rootContext->device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(streamContext->cmdAllocator.GetAddressOf()));

		_rootContext->device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			streamContext->cmdAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(streamContext->cmdList.GetAddressOf()));

		streamContext->cmdList->Close();

		_defaultStream = std::make_shared<Stream>(_rootContext, streamContext);

		Core::RootApp<AppContext>::Initialize();
	}
}
