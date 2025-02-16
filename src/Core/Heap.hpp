#pragma once
#include <memory>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>
#include <Core/Task.hpp>
#include <Core/Entity.hpp>

namespace MMPEngine::Core
{
	class Heap : public std::enable_shared_from_this<Heap>
	{
	public:
		struct Request final
		{
			std::size_t size = 128;
			std::optional<std::size_t> alignment = std::nullopt;
		};
	protected:
		class Block
		{
		public:

			struct Range final
			{
				std::size_t from;
				std::size_t to;

				std::size_t GetLength() const;
			};

			Block(std::size_t size);
			Block(const Block&) = delete;
			Block(Block&&) = delete;
			Block& operator=(const Block&) = delete;
			Block& operator=(Block&&) = delete;
			virtual ~Block();

			std::optional<Range> TryAllocate(const Request& request);
			void Release(const Range& range);
			std::size_t GetSize() const;
			bool Empty() const;
			virtual std::shared_ptr<Core::BaseEntity> GetEntity() const = 0;

		private:

			struct RangeComparer final
			{
				bool operator()(const Range& lhs, const Range& rhs) const;
			};
			void AddRange(const Range& range);
			void RemoveRange(const Range& range);

			std::set<Range, RangeComparer> _freeRanges;
			std::unordered_map<std::size_t, Range> _fromMap;
			std::unordered_map<std::size_t, Range> _toMap;
			std::size_t _size;
		};
	public:
		struct Settings final
		{
			std::size_t initialSize = 1024;
			std::size_t growthFactor = 2;
			bool removeEmptyBlocks = false;
		};
		Heap(const Settings& settings);
		Heap(const Heap&) = delete;
		Heap(Heap&&) noexcept = delete;
		Heap& operator=(const Heap&) = delete;
		Heap& operator=(Heap&&) noexcept = delete;
		virtual ~Heap();
		std::shared_ptr<Core::BaseTask> CreateTaskToInitializeBlocks();
	protected:
		struct Entry final
		{
			std::size_t blockIndex;
			Block::Range range;
		};

		virtual Entry AllocateEntry(const Request& request);
		virtual std::unique_ptr<Block> InstantiateBlock(std::size_t size) = 0;
		virtual void ReleaseEntry(const Entry& entry);

		class Handle
		{
		protected:
			Handle();
			Handle(const std::shared_ptr<Heap>& heap, const Entry& entry);
		public:
			Handle(const Handle&) = delete;
			Handle(Handle&& movableHandle) noexcept;
			Handle& operator=(const Handle&) = delete;
			Handle& operator=(Handle&& movableHandle) noexcept;
			virtual ~Handle();
		protected:
			std::optional<Entry> _entry;
			std::weak_ptr<Heap> _heap;
		};

		Settings _settings;
		std::vector<std::unique_ptr<Block>> _blocks;
	private:

		class InitBlocksTaskContext : public Core::TaskContext
		{
		public:
			std::shared_ptr<Heap> heap;
		};

		class InitBlocksTask final : public Core::BaseTask
		{
		public:
			InitBlocksTask(const std::shared_ptr<InitBlocksTaskContext>& ctx);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		private:
			std::shared_ptr<InitBlocksTaskContext> _ctx;
		};


		std::optional<std::size_t> _lastInstantiatedBlockSize = std::nullopt;
		std::unordered_set<std::uint64_t> _initializedBlockEntityIds;
	};
}
