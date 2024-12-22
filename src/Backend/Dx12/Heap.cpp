#include <Backend/Dx12/Heap.hpp>
#include <Backend/Dx12/d3dx12.h>
#include <Backend/Dx12/Entity.hpp>

namespace MMPEngine::Backend::Dx12
{
	ResourceEntityHeap::ResourceEntityHeap(const Settings& settings, const Microsoft::WRL::ComPtr<ID3D12Device>& device)
		: Core::Heap(settings.base), _re(settings.re), _device(device)
	{
	}

	std::unique_ptr<Core::Heap::Block> ResourceEntityHeap::InstantiateBlock(std::size_t size)
	{
		return std::make_unique<Block>(size, _re, _device);
	}

	ResourceEntityHeap::Block::Block(std::size_t size, const RESettings& resourceSettings, const Microsoft::WRL::ComPtr<ID3D12Device>& device) : Core::Heap::Block(size)
	{
		const auto heapProperties = CD3DX12_HEAP_PROPERTIES(resourceSettings.heapType);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
		resourceDesc.Flags = resourceSettings.flags;

		Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;

		device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			ResourceEntity::kDefaultState,
			nullptr,
			IID_PPV_ARGS(&resource));

		_entity = std::make_shared<ResourceEntityWrapper>("ResourceEntityHeap_block", resource, ResourceEntity::kDefaultState);
	}

	std::shared_ptr<ResourceEntity> ResourceEntityHeap::Block::GetEntity() const
	{
		return _entity;
	}

	ResourceEntityHeap::Handle::Handle(const std::shared_ptr<Heap>& heap, const Entry& entry, const std::shared_ptr<ResourceEntity>& blockEntity)
		: Core::Heap::Handle(heap, entry), _blockEntity(blockEntity)
	{
	}

	ResourceEntityHeap::Handle::Handle() = default;

	std::shared_ptr<ResourceEntity> ResourceEntityHeap::Handle::GetBlockEntity() const
	{
		return _blockEntity;
	}

	std::size_t ResourceEntityHeap::Handle::GetOffset() const
	{
		return _entry.value().range.from;
	}

	ResourceEntityHeap::Handle ResourceEntityHeap::Allocate(const Request& request)
	{
		const auto entry = AllocateEntry(request);
		const auto block = dynamic_cast<Block*>(_blocks[entry.blockIndex].get());
		return { shared_from_this(), entry, block->GetEntity()};
	}

}
