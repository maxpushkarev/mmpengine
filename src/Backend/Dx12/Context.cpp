#include <Backend/Dx12/Context.hpp>

namespace MMPEngine::Backend::Dx12
{
	AppContext::AppContext(const Core::AppContext::Settings& s, std::unique_ptr<Core::Math>&& m, std::unique_ptr<Core::BaseLogger>&& l) : Core::AppContext(s, std::move(m), std::move(l))
	{
	}

	StreamContext::StreamContext(
		const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& queue, 
		const Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& allocator, 
		const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& list, 
		const Microsoft::WRL::ComPtr<ID3D12Fence>& fence
	) : _cmdQueue(queue), _cmdAllocator(allocator), _cmdList(list), _fence(fence)
	{
	}

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& StreamContext::PopulateCommandsInList()
	{
		_populatedCommands = true;
		return _cmdList;
	}

}