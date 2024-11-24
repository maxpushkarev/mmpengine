#pragma once
#include <memory>
#include <optional>
#include <vector>

namespace MMPEngine::Core
{
	class BaseItemHeap : public std::enable_shared_from_this<BaseItemHeap>
	{
	protected:
		struct Entry final
		{
			std::uint32_t blockIndex;
			std::uint32_t slotIndexInBlock;
		};
		class Block
		{
		public:
			Block(std::uint32_t size);
			Block(const Block&) = delete;
			Block(Block&&) noexcept = delete;
			Block& operator=(const Block&) = delete;
			Block& operator=(Block&&) noexcept = delete;
			virtual ~Block();
			std::optional<std::uint32_t> TryAllocate();
			virtual void Release(std::uint32_t slotIndex);
		protected:
			std::vector<bool> _freeSlots;
		};
	public:
		struct Settings
		{
			std::uint32_t initialCapacity = 4;
			std::uint32_t growthFactor = 2;
		};
		class Handle
		{
		protected:
			Handle(const std::shared_ptr<BaseItemHeap>& heap, const Entry& entry);
		public:
			Handle(const Handle&) = delete;
			Handle(Handle&& movableHandle) noexcept;
			Handle& operator=(const Handle&) = delete;
			Handle& operator=(Handle&& movableHandle) noexcept;
			virtual ~Handle();
		protected:
			std::optional<Entry> _entry;
			std::weak_ptr<BaseItemHeap> _heap;
		};
	protected:
		BaseItemHeap(const Settings& settings);
		virtual ~BaseItemHeap();
		virtual Entry Allocate();
		virtual std::unique_ptr<Block> InstantiateBlock(std::uint32_t size) = 0;
		virtual void Release(const Entry& entry);

	public:
		BaseItemHeap(const BaseItemHeap&) = delete;
		BaseItemHeap(BaseItemHeap&&) noexcept = delete;
		BaseItemHeap& operator=(const BaseItemHeap&) = delete;
		BaseItemHeap& operator=(BaseItemHeap&&) noexcept = delete;
	protected:
		Settings _settings;
		std::vector<std::unique_ptr<Block>> _blocks;
	};
}
