#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include <Core/Context.hpp>
#include <Backend/Dx12/Pool.hpp>
#include <Backend/Dx12/Heap.hpp>
#include <Backend/Dx12/Wrapper.hpp>
#include <Backend/Shared/Context.hpp>
#include <wrl/client.h>

namespace MMPEngine::Backend::Dx12
{
	class GlobalContext : public Core::GlobalContext
	{
	public:
		GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m);

		Microsoft::WRL::ComPtr<ID3D12Device> device;
		Microsoft::WRL::ComPtr<IDXGIFactory> factory;

		std::shared_ptr<RTVDescriptorPool> rtvDescPool;
		std::shared_ptr<DSVDescriptorPool> dsvDescPool;
		std::shared_ptr<CBVSRVUAVDescriptorPool> cbvSrvUavShaderVisibleDescPool;
		std::shared_ptr<CBVSRVUAVDescriptorPool> cbvSrvUavShaderInVisibleDescPool;
		std::shared_ptr<ConstantBufferHeap> constantBufferEntityHeap;
	};

	class StreamContext : public Shared::StreamContext<
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>,
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>,
		std::shared_ptr<Wrapper::Fence>>
	{
	public:
		StreamContext(
			const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& queue, 
			const Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& allocator,
			const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& list,
			const std::shared_ptr<Wrapper::Fence>& fence);
	};
}
