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
		private:
			std::shared_ptr<DeviceMemoryBlock> _entity;
		};

	public:
		class Handle final : public Core::Heap::Handle
		{
		public:
			Handle();
		protected:
			Handle(const std::shared_ptr<Heap>& heap, const Entry& entry);
		};

		DeviceMemoryHeap(const Settings& settings, VkMemoryPropertyFlagBits flags);
		std::unique_ptr<Heap::Block> InstantiateBlock(std::size_t size) override;
	private:
		VkMemoryPropertyFlagBits _flags;
	};
}
