#pragma once
#include <Core/Heap.hpp>
#include <Backend/Vulkan/Memory.hpp>

namespace MMPEngine::Backend::Vulkan
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

		DeviceMemoryHeap(const Settings& settings, VkMemoryPropertyFlagBits includeFlags, VkMemoryPropertyFlagBits excludeFlags);
		Handle Allocate(const Request& request);
	protected:
		std::unique_ptr<Heap::Block> InstantiateBlock(std::size_t size) override;
	private:
		VkMemoryPropertyFlagBits _includeFlags;
		VkMemoryPropertyFlagBits _excludeFlags;
	};
}
