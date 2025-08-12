#pragma once
#include <Core/Heap.hpp>
#include <Backend/Metal/Memory.hpp>

namespace MMPEngine::Backend::Metal
{
    class DeviceMemoryHeap final : public Core::Heap
    {
    private:
        class Block final : public Core::Heap::Block
        {
        public:
            Block(const DeviceMemoryBlock::Settings& memBlockSettings);
            std::shared_ptr<Core::BaseEntity> GetEntity() const override;
            std::shared_ptr<DeviceMemoryBlock> GetEntityBlock() const;
        private:
            std::shared_ptr<DeviceMemoryBlock> _entity;
        };

    public:
        class Handle final : public Core::Heap::Handle
        {
            friend DeviceMemoryHeap;
        public:
            Handle();
            std::size_t GetOffset() const;
            std::size_t GetSize() const;
            std::shared_ptr<DeviceMemoryBlock> GetMemoryBlock() const;
        protected:
            Handle(const std::shared_ptr<Heap>& heap, const Entry& entry, const std::shared_ptr<DeviceMemoryBlock>& deviceMemoryBlock);
        private:
            std::shared_ptr<DeviceMemoryBlock> _deviceMemoryBlock;
        };

        DeviceMemoryHeap(const Settings& settings, const DeviceMemoryBlock::MTLSettings& memBlockMtlSettings);
        Handle Allocate(const Request& request);
        const DeviceMemoryBlock::MTLSettings& GetMtlSettings() const;
    protected:
        std::unique_ptr<Heap::Block> InstantiateBlock(std::size_t size) override;
    private:
        DeviceMemoryBlock::MTLSettings _memBlockMtlSettings;
    };
}

