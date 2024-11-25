#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include <Core/Context.hpp>
#include <Backend/Dx12/Heap.hpp>
#include <wrl/client.h>

namespace MMPEngine::Backend::Dx12
{
	class AppContext : public Core::AppContext
	{
	public:
		AppContext(const Core::AppContext::Settings& s);

		Microsoft::WRL::ComPtr<ID3D12Device> device;
		Microsoft::WRL::ComPtr<IDXGIFactory> factory;

		std::shared_ptr<RTVDescriptorHeap> rtvHeap;
		std::shared_ptr<DSVDescriptorHeap> dsvHeap;
		std::shared_ptr<CBVSRVUAVDescriptorHeap> cbvSrvUavShaderVisibleHeap;
		std::shared_ptr<CBVSRVUAVDescriptorHeap> cbvSrvUavShaderInVisibleHeap;
	};

	class StreamContext : public Core::StreamContext
	{
	public:
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> cmdQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList;
		Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	};
}
