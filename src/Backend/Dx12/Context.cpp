#include <Backend/Dx12/Context.hpp>

namespace MMPEngine::Backend::Dx12
{
	GlobalContext::GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m) : Core::GlobalContext(s, std::move(m))
	{
	}

	StreamContext::StreamContext(
		const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& queue, 
		const Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& allocator, 
		const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& list, 
		const std::shared_ptr<Wrapper::Fence>& fence
	) :Shared::StreamContext<
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>,
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>,
		std::shared_ptr<Wrapper::Fence>>(queue, allocator, list, fence)
	{
	}
}