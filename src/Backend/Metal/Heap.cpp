#include <Backend/Metal/Heap.hpp>

namespace MMPEngine::Backend::Metal
{
    DeviceMemoryHeap::DeviceMemoryHeap(const Settings& settings, const DeviceMemoryBlock::MTLSettings& memBlockMtlSettings) : Core::Heap(settings), _memBlockMtlSettings(memBlockMtlSettings)
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
        auto newSettings = DeviceMemoryBlock::Settings {size, _memBlockMtlSettings};
        return std::make_unique<Block>(newSettings);
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


    DeviceMemoryHeap::Block::Block(const DeviceMemoryBlock::Settings& memBlockSettings) : Core::Heap::Block(memBlockSettings.size), _entity(std::make_shared<DeviceMemoryBlock>(memBlockSettings))
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
