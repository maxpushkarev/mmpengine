#pragma once
#include <memory>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

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
		};
		Heap(const Settings& settings);
		Heap(const Heap&) = delete;
		Heap(Heap&&) noexcept = delete;
		Heap& operator=(const Heap&) = delete;
		Heap& operator=(Heap&&) noexcept = delete;
		virtual ~Heap();
	protected:
		struct Entry final
		{
			std::size_t blockIndex;
			Block::Range range;
		};
		Settings _settings;
		virtual Entry Allocate(const Request& request);
		virtual std::unique_ptr<Block> InstantiateBlock(std::size_t size);
		virtual void Release(const Entry& entry);

		std::vector<std::unique_ptr<Block>> _blocks;
	};
}
