#include <Core/Heap.hpp>
#include <cassert>

namespace MMPEngine::Core
{
	Heap::Heap(const Settings& settings) : _settings(settings)
	{
	}
	Heap::~Heap() = default;


	Heap::Block::Block(std::size_t size)
	{
		AddRange({0, size - 1});
	}
	Heap::Block::~Block() = default;

	std::size_t Heap::Block::Range::GetLength() const
	{
		assert(to >= from);
		return to - from + 1;
	}

	bool Heap::Block::RangeComparer::operator()(const Range& lhs, const Range& rhs) const
	{
		const auto lhsLength = lhs.GetLength();
		const auto rhsLength = rhs.GetLength();

		if(lhsLength != rhsLength)
		{
			return lhsLength < rhsLength;
		}

		return lhs.from < rhs.from;
	}

	void Heap::Block::AddRange(const Range& range)
	{
		assert(range.to > range.from);
		assert(_freeRanges.emplace(range).second);
		assert(_fromMap.emplace(range.from, range).second);
		assert(_toMap.emplace(range.to, range).second);
	}

	void Heap::Block::RemoveRange(const Range& range)
	{
		assert(range.to > range.from);
		assert(_freeRanges.erase(range) > 0);
		assert(_fromMap.erase(range.from) > 0);
		assert(_toMap.erase(range.to) > 0);
	}


	std::optional<Heap::Block::Range> Heap::Block::TryAllocate(const Request& request)
	{
		const Range key = {0, request.size - 1};
		auto it = _freeRanges.lower_bound(key);

		std::optional<Range> rangePlaceholder = std::nullopt;
		std::optional<Range> res = std::nullopt;

		while (it != _freeRanges.cend())
		{
			if(request.alignment.has_value())
			{
				const auto requiredAlignment = request.alignment.value();
				const auto mod = it->from % requiredAlignment;

				if(mod == 0)
				{
					if (it->GetLength() >= request.size)
					{
						rangePlaceholder = *it;
						res = { it->from, it->from + request.size - 1 };
						break;
					}
				}
				else
				{
					const auto f = it->from + (requiredAlignment - mod);
					const auto t = f + request.size - 1;

					if(
						f >= it->from && f <= it->to && 
						t >= it->from && t <= it->to
						)
					{
						rangePlaceholder = *it;
						res = {f, t};
						break;
					}
				}
			}
			else
			{
				if(it->GetLength() >= request.size)
				{
					rangePlaceholder = *it;
					res = {it->from, it->from + request.size - 1};
					break;
				}
			}

			++it;
		}

		if(rangePlaceholder.has_value())
		{
			assert(res.has_value());

			const auto placeholderRange = rangePlaceholder.value();
			const auto allocatedRange = res.value();

			RemoveRange(placeholderRange);

			const Range left = {placeholderRange.from, allocatedRange.from - 1};
			const Range right = {allocatedRange.to + 1, placeholderRange.to};

			if(left.to > left.from)
			{
				AddRange(left);
			}

			if (right.to > right.from)
			{
				AddRange(right);
			}
		}

		return res;
	}

	std::size_t Heap::Block::GetSize() const
	{
		return _size;
	}


	void Heap::Block::Release(const Range& range)
	{
		Range newRange = range;

		const auto adjacentTo = newRange.from - 1;
		const auto adjacentFrom = newRange.to + 1;

		if(_toMap.find(adjacentTo) != _toMap.cend())
		{
			const auto r = _toMap.at(adjacentTo);
			RemoveRange(r);
			newRange.from = r.from;
		}

		if(_fromMap.find(adjacentFrom) != _fromMap.cend())
		{
			const auto r = _fromMap.at(adjacentFrom);
			RemoveRange(r);
			newRange.to = r.to;
		}

		AddRange(newRange);
	}

	Heap::Entry Heap::Allocate(const Request& request)
	{
		assert(request.size > 0);

		std::size_t blockIndex = 0;

		while(true)
		{
			Block* blockPtr = nullptr;

			if(blockIndex == _blocks.size())
			{
				auto newBlockSize = std::max(_settings.initialSize, request.size);

				if(!_blocks.empty())
				{
					newBlockSize = _blocks.back()->GetSize() * _settings.growthFactor;
				}

				_blocks.emplace_back(InstantiateBlock(newBlockSize));
				blockPtr = _blocks.back().get();
			}
			else
			{
				blockPtr = _blocks.at(blockIndex).get();
			}

			const auto range = blockPtr->TryAllocate(request);

			if(range.has_value())
			{
				assert(!request.alignment.has_value() || ((range.value().from % request.alignment.value()) == 0));
				assert(range.value().GetLength() == request.size);

				return { static_cast<std::size_t>(blockIndex), range.value() };
			}

			++ blockIndex;
		}
	}

	void Heap::Release(const Entry& entry)
	{
		_blocks.at(entry.blockIndex)->Release(entry.range);
	}

}