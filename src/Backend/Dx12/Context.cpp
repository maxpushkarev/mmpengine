#include <Backend/Dx12/Context.hpp>
#include <Backend/Dx12/Pool.hpp>

namespace MMPEngine::Backend::Dx12
{
	GlobalContext::GlobalContext(const Core::GlobalContext::Settings& s, const Core::GlobalContext::Environment& env, std::unique_ptr<Core::Math>&& m) : Core::GlobalContext(s, env, std::move(m))
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