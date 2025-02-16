#include <Backend/Vulkan/Heap.hpp>

namespace MMPEngine::Backend::Vulkan
{
	DeviceMemoryHeap::DeviceMemoryHeap(const Settings& settings) : Core::Heap(settings)
	{
	}

	std::unique_ptr<Core::Heap::Block> DeviceMemoryHeap::InstantiateBlock(std::size_t size)
	{
		return std::make_unique<Block>(size);
	}

	DeviceMemoryHeap::Handle::Handle() = default;

	DeviceMemoryHeap::Handle::Handle(const std::shared_ptr<Heap>& heap, const Entry& entry) : Core::Heap::Handle(heap, entry)
	{
	}

	DeviceMemoryHeap::Block::Block(std::size_t size) : Core::Heap::Block(size), _entity(std::make_shared<DeviceMemoryBlock>(DeviceMemoryBlock::Settings {size}))
	{
	}

	std::shared_ptr<Core::BaseEntity> DeviceMemoryHeap::Block::GetEntity() const
	{
		return _entity;
	}
}