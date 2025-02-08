#include <Backend/Dx12/Context.hpp>

namespace MMPEngine::Backend::Dx12
{
	GlobalContext::GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m) : Core::GlobalContext(s, std::move(m))
	{
	}

	StreamContext::StreamContext(
		const std::shared_ptr<GlobalContext>& globalContext,
		const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& queue, 
		const Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& allocator, 
		const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& list, 
		const Microsoft::WRL::ComPtr<ID3D12Fence>& fence
	) :Shared::StreamContext<
		GlobalContext,
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>,
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>,
		Microsoft::WRL::ComPtr<ID3D12Fence>>(globalContext, queue, allocator, list, fence)
	{
	}
}