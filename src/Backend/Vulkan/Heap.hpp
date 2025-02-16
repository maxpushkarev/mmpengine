#pragma once
#include <Core/Heap.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class DeviceMemoryHeap final : public Core::Heap
	{
	private:
		class Block : public Core::Heap::Block
		{
		public:
			Block(std::size_t size);
			Block(const Block&) = delete;
			Block(Block&&) noexcept = delete;
			Block& operator=(const Block&) = delete;
			Block& operator=(Block&&) noexcept = delete;
			~Block() override;
		};

	public:
		class Handle : public Core::Heap::Handle
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
