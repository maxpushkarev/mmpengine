#pragma once
#include <Core/Heap.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class DeviceMemoryHeap final : public Core::Heap
	{
	private:
		class Block final : public Core::Heap::Block
		{
		public:
			Block(std::size_t size);
			std::shared_ptr<Core::BaseEntity> GetEntity() const override;
		};

	public:
		class Handle final : public Core::Heap::Handle
		{
		public:
			Handle();
		protected:
			Handle(const std::shared_ptr<Heap>& heap, const Entry& entry);
		};

		DeviceMemoryHeap(const Settings& settings);
		std::unique_ptr<Heap::Block> InstantiateBlock(std::size_t size) override;
	};
}
