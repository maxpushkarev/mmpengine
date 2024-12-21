#pragma once
#include <memory>
#include <optional>
#include <set>
#include <vector>

namespace MMPEngine::Core
{
	class Heap : public std::enable_shared_from_this<Heap>
	{
	public:
		struct Request final
		{
			std::uint32_t size = 128;
			std::optional<std::uint32_t> alignment = std::nullopt;
		};
	protected:
		class Block
		{
		public:

			struct Range final
			{
				std::uint32_t from;
				std::uint32_t to;

				std::uint32_t GetLength() const;
			};

			Block(std::uint32_t size);
			Block(const Block&) = delete;
			Block(Block&&) = delete;
			Block& operator=(const Block&) = delete;
			Block& operator=(Block&&) = delete;
			virtual ~Block();

			std::optional<Range> TryAllocate(const Request& request);
			void Release(const Range& range);

		private:
			struct RangeComparer final
			{
				bool operator()(const Range& lhs, const Range& rhs) const;
			};
			void AddRange(const Range& range);
			void RemoveRange(const Range& range);

			std::set<Range, RangeComparer> _freeRanges;
		};
	public:
		struct Settings final
		{
			std::uint32_t initialSize = 1024;
			std::uint32_t growthFactor = 2;
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
			std::uint32_t blockIndex;
			Block::Range range;
		};
		Settings _settings;
		virtual Entry Allocate(const Request& request);
		virtual std::unique_ptr<Block> InstantiateBlock(std::uint32_t size) = 0;
		virtual void Release(const Entry& entry);

		std::vector<std::unique_ptr<Block>> _blocks;
	};
}
