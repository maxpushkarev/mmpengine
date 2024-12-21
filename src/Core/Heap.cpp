#include <Core/Heap.hpp>
#include <cassert>

namespace MMPEngine::Core
{
	Heap::Heap(const Settings& settings) : _settings(settings)
	{
	}
	Heap::~Heap() = default;


	Heap::Block::Block(std::uint32_t size)
	{
		AddRange({0, size - 1});
	}
	Heap::Block::~Block() = default;

	std::uint32_t Heap::Block::Range::GetLength() const
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

		const auto addFreeRangeResult = _freeRanges.emplace(range);
		assert(addFreeRangeResult.second);
	}

	void Heap::Block::RemoveRange(const Range& range)
	{
		assert(range.to > range.from);

		const auto freeRangeEraseResult = _freeRanges.erase(range);
		assert(freeRangeEraseResult > 0);
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

	void Heap::Block::Release(const Range& range)
	{
	}

	Heap::Entry Heap::Allocate(const Request& request)
	{
		assert(request.size > 0);

		auto newBlockSize = std::max(_settings.initialSize, request.size);
		std::size_t blockIndex = 0;

		while(true)
		{
			Block* blockPtr = nullptr;

			if(blockIndex == _blocks.size())
			{
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

				return { static_cast<std::uint32_t>(blockIndex), range.value() };
			}

			newBlockSize *= _settings.growthFactor;
			++ blockIndex;
		}
	}

	void Heap::Release(const Entry& entry)
	{
		_blocks.at(entry.blockIndex)->Release(entry.range);
	}

}