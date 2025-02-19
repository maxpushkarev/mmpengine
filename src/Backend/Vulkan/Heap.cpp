#include <Backend/Vulkan/Heap.hpp>

namespace MMPEngine::Backend::Vulkan
{
	DeviceMemoryHeap::DeviceMemoryHeap(const Settings& settings, VkMemoryPropertyFlagBits includeFlags, VkMemoryPropertyFlagBits excludeFlags) : Core::Heap(settings), _includeFlags(includeFlags), _excludeFlags(excludeFlags)
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
		return std::make_unique<Block>(DeviceMemoryBlock::Settings {size, _includeFlags, _excludeFlags});
	}

	DeviceMemoryHeap::Handle::Handle() = default;

	DeviceMemoryHeap::Handle::Handle(const std::shared_ptr<Heap>& heap, const Entry& entry, const std::shared_ptr<DeviceMemoryBlock>& deviceMemoryBlock) : Core::Heap::Handle(heap, entry), _deviceMemoryBlock(deviceMemoryBlock)
	{
	}

	std::size_t DeviceMemoryHeap::Handle::GetOffset() const
	{
		return _entry->range.from;
	}

	std::shared_ptr<DeviceMemoryBlock> DeviceMemoryHeap::Handle::GetMemoryBlock() const
	{
		return _deviceMemoryBlock;
	}


	DeviceMemoryHeap::Block::Block(const DeviceMemoryBlock::Settings& memBlockSettings) : Core::Heap::Block(memBlockSettings.byteSize), _entity(std::make_shared<DeviceMemoryBlock>(memBlockSettings))
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

}