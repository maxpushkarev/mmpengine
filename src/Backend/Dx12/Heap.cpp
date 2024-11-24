#include <Backend/Dx12/Heap.hpp>

namespace MMPEngine::Backend::Dx12
{
	BaseDescriptorHeap::BaseDescriptorHeap(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Settings& settings)
		: Core::BaseItemHeap(settings.base),
		_nativeSettings(settings.native),
		_device(device)
	{
		_incrementSize = device->GetDescriptorHandleIncrementSize(_nativeSettings.type);
	}

	std::unique_ptr<Core::BaseItemHeap::Block> BaseDescriptorHeap::InstantiateBlock(std::uint32_t size)
	{
		return std::make_unique<Block>(size, _device, _nativeSettings);
	}

	BaseDescriptorHeap::Handle BaseDescriptorHeap::Allocate()
	{
		return {std::dynamic_pointer_cast<BaseDescriptorHeap>(shared_from_this()), AllocateEntry() };
	}

	BaseDescriptorHeap::Handle::Handle(const std::shared_ptr<BaseDescriptorHeap>& descHeap, const Entry& entry)
		: Core::BaseItemHeap::Handle(descHeap, entry), _descHeap(descHeap)
	{
	}

	BaseDescriptorHeap::Block::Block(std::uint32_t size, const Microsoft::WRL::ComPtr<ID3D12Device>& device, const NativeSettings& nativeSettings) : Core::BaseItemHeap::Block(size)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc {};
		heapDesc.NumDescriptors = size;
		heapDesc.Type = nativeSettings.type;
		heapDesc.Flags = nativeSettings.flags;
		heapDesc.NodeMask = 0;

		device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_native));
	}

	RTVDescriptorHeap::RTVDescriptorHeap(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Core::BaseItemHeap::Settings& baseSettings, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
		: DescriptorHeap(device, baseSettings, flags)
	{
	}

	DSVDescriptorHeap::DSVDescriptorHeap(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Core::BaseItemHeap::Settings& baseSettings, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
		: DescriptorHeap(device, baseSettings, flags)
	{
	}

	CBVSRVUAVDescriptorHeap::CBVSRVUAVDescriptorHeap(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Core::BaseItemHeap::Settings& baseSettings, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
		: DescriptorHeap(device, baseSettings, flags)
	{
	}
}