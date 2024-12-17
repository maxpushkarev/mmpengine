#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include <Core/Context.hpp>
#include <Backend/Dx12/Heap.hpp>
#include <wrl/client.h>

namespace MMPEngine::Backend::Dx12
{
	class GlobalContext : public Core::GlobalContext
	{
	public:
		GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m, std::unique_ptr<Core::BaseLogger>&& l);

		Microsoft::WRL::ComPtr<ID3D12Device> device;
		Microsoft::WRL::ComPtr<IDXGIFactory> factory;

		std::shared_ptr<RTVDescriptorHeap> rtvHeap;
		std::shared_ptr<DSVDescriptorHeap> dsvHeap;
		std::shared_ptr<CBVSRVUAVDescriptorHeap> cbvSrvUavShaderVisibleHeap;
		std::shared_ptr<CBVSRVUAVDescriptorHeap> cbvSrvUavShaderInVisibleHeap;
	};

	class Stream;

	class StreamContext : public Core::StreamContext
	{
		friend class Stream;

	public:
		StreamContext(
			const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& queue, 
			const Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& allocator,
			const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& list,
			const Microsoft::WRL::ComPtr<ID3D12Fence>& fence);

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& PopulateCommandsInList();
	private:

		bool _populatedCommands = false;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> _cmdQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _cmdAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList;
		Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
	};
}
