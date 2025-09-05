#include <Backend/Dx12/Heap.hpp>
#include <Backend/Dx12/d3dx12.h>
#include <Backend/Dx12/Entity.hpp>
#include <Backend/Dx12/Buffer.hpp>

namespace MMPEngine::Backend::Dx12
{
	DeviceMemoryHeap::DeviceMemoryHeap(const Settings& settings, const D3D12_HEAP_DESC& desc) : Core::Heap(settings), _desc(desc)
	{
	}

	DeviceMemoryHeap::Handle DeviceMemoryHeap::Allocate(const Request& request)
	{
		const auto entry = AllocateEntry(request);
		const auto block = dynamic_cast<Block*>(_blocks[entry.blockIndex].get());
		return { shared_from_this(), entry, block->GetEntityBlock() };
	}

	std::unique_ptr<Core::Heap::Block> DeviceMemoryHeap::InstantiateBlock(std::size_t size)
	{
		auto newDesc = _desc;
		newDesc.SizeInBytes = static_cast<std::uint64_t>(size);
		return std::make_unique<Block>(newDesc);
	}

	DeviceMemoryHeap::Handle::Handle() = default;

	DeviceMemoryHeap::Handle::Handle(const std::shared_ptr<Heap>& heap, const Entry& entry, const std::shared_ptr<DeviceMemoryBlock>& deviceMemoryBlock) : Core::Heap::Handle(heap, entry), _deviceMemoryBlock(deviceMemoryBlock)
	{
	}

	std::size_t DeviceMemoryHeap::Handle::GetOffset() const
	{
		return _entry->range.from;
	}

	std::size_t DeviceMemoryHeap::Handle::GetSize() const
	{
		return _entry->range.GetLength();
	}

	std::shared_ptr<DeviceMemoryBlock> DeviceMemoryHeap::Handle::GetMemoryBlock() const
	{
		return _deviceMemoryBlock;
	}


	DeviceMemoryHeap::Block::Block(const D3D12_HEAP_DESC& desc) : Core::Heap::Block(static_cast<std::size_t>(desc.SizeInBytes)), _entity(std::make_shared<DeviceMemoryBlock>(desc))
	{
	}

	std::shared_ptr<Core::BaseEntity> DeviceMemoryHeap::Block::GetEntity() const
	{
		return _entity;
	}

	std::shared_ptr<DeviceMemoryBlock> DeviceMemoryHeap::Block::GetEntityBlock() const
	{
		return _entity;
	}



	ConstantBufferHeap::ConstantBufferHeap(const Settings& settings)
		: Core::Heap(settings)
	{
	}

	std::unique_ptr<Core::Heap::Block> ConstantBufferHeap::InstantiateBlock(std::size_t size)
	{
		return std::make_unique<Block>(size);
	}

	ConstantBufferHeap::Block::Block(std::size_t size) : Core::Heap::Block(size)
	{
		_upload = std::make_shared<UploadBuffer>(Core::Buffer::Settings {
			size, "constant_buffer_block_in_heap"
		});
	}

	std::shared_ptr<UploadBuffer> ConstantBufferHeap::Block::GetUploadBuffer() const
	{
		return _upload;
	}

	std::shared_ptr<Core::BaseEntity> ConstantBufferHeap::Block::GetEntity() const
	{
		return _upload;
	}

	ConstantBufferHeap::Handle::Handle(const std::shared_ptr<Heap>& heap, const Entry& entry, const std::shared_ptr<UploadBuffer>& blockEntity)
		: Core::Heap::Handle(heap, entry), _uploadBlock(blockEntity)
	{
	}

	ConstantBufferHeap::Handle::Handle() = default;

	std::shared_ptr<UploadBuffer> ConstantBufferHeap::Handle::GetUploadBlock() const
	{
		return _uploadBlock;
	}

	std::size_t ConstantBufferHeap::Handle::GetOffset() const
	{
		return _entry.value().range.from;
	}

	ConstantBufferHeap::Handle ConstantBufferHeap::Allocate(const Request& request)
	{
		const auto entry = AllocateEntry(request);
		const auto block = dynamic_cast<Block*>(_blocks[entry.blockIndex].get());
		return { shared_from_this(), entry, block->GetUploadBuffer()};
	}
}
