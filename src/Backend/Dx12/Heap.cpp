#include <algorithm>
#include <cassert>
#include <iterator>
#include <stdexcept>
#include <Backend/Dx12/Heap.hpp>
#include <Backend/Dx12/d3dx12.h>

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
		ResetCache();
		return std::make_unique<Block>(size, _device, _nativeSettings);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE BaseDescriptorHeap::GetNativeCPUDescriptorHandle(const Entry& entry) const
	{
		RebuildCacheIfNeed();

		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			_nativeBlocksCache.value().at(entry.blockIndex)->GetCPUDescriptorHandleForHeapStart(),
			static_cast<std::int32_t>(entry.slotIndexInBlock),
			_incrementSize);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptorHeap::GetNativeGPUDescriptorHandle(const Entry& entry) const
	{
		RebuildCacheIfNeed();


		return CD3DX12_GPU_DESCRIPTOR_HANDLE(
			_nativeBlocksCache.value().at(entry.blockIndex)->GetGPUDescriptorHandleForHeapStart(),
			static_cast<std::int32_t>(entry.slotIndexInBlock),
			_incrementSize);
	}

	BaseDescriptorHeap::Handle BaseDescriptorHeap::Allocate()
	{
		return {std::dynamic_pointer_cast<BaseDescriptorHeap>(shared_from_this()), AllocateEntry() };
	}

	void BaseDescriptorHeap::ResetCache()
	{
		_nativeBlocksCache.reset();
	}

	void BaseDescriptorHeap::RebuildCacheIfNeed() const
	{
		if(!_nativeBlocksCache.has_value())
		{
			std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> descHeaps {};
			descHeaps.resize(_blocks.size(), nullptr);

			std::transform(_blocks.cbegin(), _blocks.cend(), descHeaps.begin(), [](const auto& block)
			{
				const auto rawBlockPtr = block.get();
				const auto heapBlockRawPtr = dynamic_cast<Block*>(rawBlockPtr);
				assert(heapBlockRawPtr != nullptr);
				return heapBlockRawPtr->native;
			});

			_nativeBlocksCache.emplace(std::move(descHeaps));
		}
	}

	BaseDescriptorHeap::Handle::Handle() = default;

	D3D12_CPU_DESCRIPTOR_HANDLE BaseDescriptorHeap::Handle::GetCPUDescriptorHandle() const
	{
		if (_entry.has_value())
		{
			if (const auto heap = _descHeap.lock())
			{
				return heap->GetNativeCPUDescriptorHandle(_entry.value());
			}

			return {};
		}
		throw std::logic_error("bad access to heap handle");
	}

	D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptorHeap::Handle::GetGPUDescriptorHandle() const
	{
		if (_entry.has_value())
		{
			if (const auto heap = _descHeap.lock())
			{
				return heap->GetNativeGPUDescriptorHandle(_entry.value());
			}

			return {};
		}
		throw std::logic_error("bad access to heap handle");
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

		device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&native));
	}

	RTVDescriptorHeap::RTVDescriptorHeap(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Core::BaseItemHeap::Settings& baseSettings)
		: DescriptorHeap(device, baseSettings, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)
	{
	}

	DSVDescriptorHeap::DSVDescriptorHeap(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Core::BaseItemHeap::Settings& baseSettings)
		: DescriptorHeap(device, baseSettings, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)
	{
	}

	CBVSRVUAVDescriptorHeap::CBVSRVUAVDescriptorHeap(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Core::BaseItemHeap::Settings& baseSettings, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
		: DescriptorHeap(device, baseSettings, flags)
	{
	}
}