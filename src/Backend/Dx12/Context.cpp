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
		const Microsoft::WRL::ComPtr<ID3D12Fence>& fence
	) : _cmdQueue(queue), _cmdAllocator(allocator), _cmdList(list), _fence(fence)
	{
	}

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& StreamContext::PopulateCommandsInList()
	{
		_commandsPopulated = true;
		return _cmdList;
	}

}