#pragma once
#include <memory>
#include <optional>
#include <vector>

namespace MMPEngine::Core
{
	class Pool : public std::enable_shared_from_this<Pool>
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
			virtual std::optional<std::uint32_t> TryAllocateSlot();
			virtual void Release(std::uint32_t slotIndex);
		protected:
			std::vector<bool> _freeSlots;
		};
	public:
		struct Settings final
		{
			std::uint32_t initialCapacity = 4;
			std::uint32_t growthFactor = 2;
		};
		class Handle
		{
		protected:
			Handle();
			Handle(const std::shared_ptr<Pool>& heap, const Entry& entry);
		public:
			Handle(const Handle&) = delete;
			Handle(Handle&& movableHandle) noexcept;
			Handle& operator=(const Handle&) = delete;
			Handle& operator=(Handle&& movableHandle) noexcept;
			virtual ~Handle();
		protected:
			std::optional<Entry> _entry;
			std::weak_ptr<Pool> _heap;
		};
	protected:
		Pool(const Settings& settings);
		virtual ~Pool();
		virtual Entry AllocateEntry();
		virtual std::unique_ptr<Block> InstantiateBlock(std::uint32_t size) = 0;
		virtual void ReleaseEntry(const Entry& entry);

	public:
		Pool(const Pool&) = delete;
		Pool(Pool&&) noexcept = delete;
		Pool& operator=(const Pool&) = delete;
		Pool& operator=(Pool&&) noexcept = delete;
	protected:
		Settings _settings;
		std::vector<std::unique_ptr<Block>> _blocks;
	};
}
