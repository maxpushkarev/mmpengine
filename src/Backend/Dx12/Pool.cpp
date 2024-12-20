#include <algorithm>
#include <cassert>
#include <iterator>
#include <stdexcept>
#include <Backend/Dx12/Pool.hpp>
#include <Backend/Dx12/d3dx12.h>

namespace MMPEngine::Backend::Dx12
{
	BaseDescriptorPool::BaseDescriptorPool(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Settings& settings)
		: Core::Pool(settings.base),
		_nativeSettings(settings.native),
		_device(device)
	{
		_incrementSize = device->GetDescriptorHandleIncrementSize(_nativeSettings.type);
	}

	std::unique_ptr<Core::Pool::Block> BaseDescriptorPool::InstantiateBlock(std::uint32_t size)
	{
		ResetCache();
		return std::make_unique<Block>(size, _device, _nativeSettings);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE BaseDescriptorPool::GetNativeCPUDescriptorHandle(const Entry& entry) const
	{
		RebuildCacheIfNeed();

		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			_nativeBlocksCache.value().at(entry.blockIndex)->GetCPUDescriptorHandleForHeapStart(),
			static_cast<std::int32_t>(entry.slotIndexInBlock),
			_incrementSize);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptorPool::GetNativeGPUDescriptorHandle(const Entry& entry) const
	{
		assert(_nativeSettings.flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		RebuildCacheIfNeed();


		return CD3DX12_GPU_DESCRIPTOR_HANDLE(
			_nativeBlocksCache.value().at(entry.blockIndex)->GetGPUDescriptorHandleForHeapStart(),
			static_cast<std::int32_t>(entry.slotIndexInBlock),
			_incrementSize);
	}

	BaseDescriptorPool::Handle BaseDescriptorPool::Allocate()
	{
		return {std::dynamic_pointer_cast<BaseDescriptorPool>(shared_from_this()), AllocateEntry() };
	}

	void BaseDescriptorPool::CollectNativeBlocks(std::vector<ID3D12DescriptorHeap*>& nativeHeaps) const
	{
		RebuildCacheIfNeed();

		const auto& cached = _nativeBlocksCache.value();
		for(const auto& heapPtr : cached)
		{
			nativeHeaps.push_back(heapPtr.Get());
		}
	}


	void BaseDescriptorPool::ResetCache()
	{
		_nativeBlocksCache.reset();
	}

	void BaseDescriptorPool::RebuildCacheIfNeed() const
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

	BaseDescriptorPool::Handle::Handle() = default;

	const D3D12_CPU_DESCRIPTOR_HANDLE& BaseDescriptorPool::Handle::GetCPUDescriptorHandle() const
	{
		return _cpuHandle;
	}

	const D3D12_GPU_DESCRIPTOR_HANDLE& BaseDescriptorPool::Handle::GetGPUDescriptorHandle() const
	{
		assert(_heapFlags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		return _gpuHandle;
	}

	BaseDescriptorPool::Handle::Handle(const std::shared_ptr<BaseDescriptorPool>& descHeap, const Entry& entry)
		: Core::Pool::Handle(descHeap, entry), _descHeap(descHeap), _heapFlags(descHeap->_nativeSettings.flags)
	{
		if (_entry.has_value())
		{
			if (const auto heap = _descHeap.lock())
			{
				if(_heapFlags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
				{
					_gpuHandle = heap->GetNativeGPUDescriptorHandle(_entry.value());
				}
				_cpuHandle = heap->GetNativeCPUDescriptorHandle(_entry.value());
			}
		}
	}

	BaseDescriptorPool::Block::Block(std::uint32_t size, const Microsoft::WRL::ComPtr<ID3D12Device>& device, const NativeSettings& nativeSettings) : Core::Pool::Block(size)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc {};
		heapDesc.NumDescriptors = size;
		heapDesc.Type = nativeSettings.type;
		heapDesc.Flags = nativeSettings.flags;
		heapDesc.NodeMask = 0;

		device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&native));
	}

	RTVDescriptorPool::RTVDescriptorPool(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Core::Pool::Settings& baseSettings)
		: DescriptorPool(device, baseSettings, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)
	{
	}

	DSVDescriptorPool::DSVDescriptorPool(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Core::Pool::Settings& baseSettings)
		: DescriptorPool(device, baseSettings, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)
	{
	}

	CBVSRVUAVDescriptorPool::CBVSRVUAVDescriptorPool(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Core::Pool::Settings& baseSettings, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
		: DescriptorPool(device, baseSettings, flags)
	{
	}
}