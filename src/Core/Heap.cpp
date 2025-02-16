#include <Core/Heap.hpp>
#include <cassert>

namespace MMPEngine::Core
{
	Heap::Heap(const Settings& settings) : _settings(settings)
	{
	}
	Heap::~Heap() = default;


	Heap::Block::Block(std::size_t size) : _size(size)
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
		assert(range.to >= range.from);

		const auto res1 = _freeRanges.emplace(range).second;
		const auto res2 = _fromMap.emplace(range.from, range).second;
		const auto res3 = _toMap.emplace(range.to, range).second;

		assert(res1);
		assert(res2);
		assert(res3);
	}

	void Heap::Block::RemoveRange(const Range& range)
	{
		assert(range.to >= range.from);

		const auto res1 = _freeRanges.erase(range) > 0;
		const auto res2 = _fromMap.erase(range.from) > 0;
		const auto res3 = _toMap.erase(range.to) > 0;

		assert(res1);
		assert(res2);
		assert(res3);
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
			assert(res.value().from >= rangePlaceholder.value().from && res.value().from <= rangePlaceholder.value().to);
			assert(res.value().to >= rangePlaceholder.value().from && res.value().to <= rangePlaceholder.value().to);
			assert(res.value().to >= res.value().from);

			const auto placeholderRange = rangePlaceholder.value();
			const auto allocatedRange = res.value();

			RemoveRange(placeholderRange);

			if(allocatedRange.from > 0)
			{
				const Range left = { placeholderRange.from, allocatedRange.from - 1 };
				if (left.to >= left.from)
				{
					AddRange(left);
				}
			}

			const Range right = { allocatedRange.to + 1, placeholderRange.to };
			if (right.to >= right.from)
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

	bool Heap::Block::Empty() const
	{
		if(_freeRanges.size() > 1)
		{
			return false;
		}

		const auto& singleRange = _freeRanges.cbegin();
		return (singleRange->from == 0 && singleRange->to == _size - 1);
	}

	void Heap::Block::Release(const Range& range)
	{
		Range newRange = range;

		if(newRange.from > 0)
		{
			const auto adjacentTo = newRange.from - 1;
			if (_toMap.find(adjacentTo) != _toMap.cend())
			{
				const auto r = _toMap.at(adjacentTo);
				RemoveRange(r);
				newRange.from = r.from;
			}
		}

		const auto adjacentFrom = newRange.to + 1;
		if(_fromMap.find(adjacentFrom) != _fromMap.cend())
		{
			const auto r = _fromMap.at(adjacentFrom);
			RemoveRange(r);
			newRange.to = r.to;
		}

		AddRange(newRange);
	}

	Heap::Entry Heap::AllocateEntry(const Request& request)
	{
		assert(request.size > 0);

		for(std::size_t blockIndex = 0; blockIndex < _blocks.size(); ++blockIndex)
		{
			auto& blockPtr = _blocks.at(blockIndex);

			if (!blockPtr)
			{
				continue;
			}

			const auto range = blockPtr->TryAllocate(request);
			if (range.has_value())
			{
				assert(!request.alignment.has_value() || ((range.value().from % request.alignment.value()) == 0));
				assert(range.value().GetLength() == request.size);

				return { static_cast<std::size_t>(blockIndex), range.value() };
			}

			if(_settings.removeEmptyBlocks && blockPtr->Empty())
			{
				blockPtr = nullptr;
			}
		}

		{
			std::size_t blockIndex = 0;
			while(true)
			{
				Block* blockPtr = nullptr;

				if(blockIndex == _blocks.size() || !_blocks.at(blockIndex))
				{
					auto newBlockSize = std::max(_settings.initialSize, request.size);

					if(_lastInstantiatedBlockSize.has_value())
					{
						newBlockSize = std::max(_lastInstantiatedBlockSize.value() * _settings.growthFactor, request.size);
					}

					_lastInstantiatedBlockSize = newBlockSize;

					if(blockIndex == _blocks.size())
					{
						_blocks.emplace_back(InstantiateBlock(newBlockSize));
					}
					else
					{
						_blocks.at(blockIndex) = InstantiateBlock(newBlockSize);
					}
				}

				blockPtr = _blocks.at(blockIndex).get();
				const auto range = blockPtr->TryAllocate(request);

				if(range.has_value())
				{
					assert(!request.alignment.has_value() || ((range.value().from % request.alignment.value()) == 0));
					assert(range.value().GetLength() == request.size);

					return { static_cast<std::size_t>(blockIndex), range.value() };
				}

				if (_settings.removeEmptyBlocks && blockPtr->Empty())
				{
					_blocks.at(blockIndex) = nullptr;
				}

				++ blockIndex;
			}
		}
	}

	void Heap::ReleaseEntry(const Entry& entry)
	{
		auto& b = _blocks.at(entry.blockIndex);
		b->Release(entry.range);
		if (_settings.removeEmptyBlocks && b->Empty())
		{
			b = nullptr;
		}
	}

	std::unique_ptr<Heap::Block> Heap::InstantiateBlock(std::size_t size)
	{
		return std::make_unique<Block>(size);
	}


	Heap::Handle::Handle() = default;

	Heap::Handle::Handle(const std::shared_ptr<Heap>& pool, const Entry& entry) : _entry(entry), _heap(pool)
	{
	}

	Heap::Handle::Handle(Handle&& movableHandle) noexcept
		: _entry{ movableHandle._entry }, _heap{ std::move(movableHandle._heap) }
	{
		movableHandle._heap.reset();
		movableHandle._entry = std::nullopt;
	}

	Heap::Handle& Heap::Handle::operator=(Handle&& movableHandle) noexcept
	{
		if (this != &movableHandle)
		{
			_heap = std::move(movableHandle._heap);
			_entry = movableHandle._entry;

			movableHandle._heap.reset();
			movableHandle._entry = std::nullopt;
		}
		return *this;
	}

	Heap::Handle::~Handle()
	{
		if (_entry.has_value())
		{
			if (const auto heapStrongRef = _heap.lock())
			{
				heapStrongRef->ReleaseEntry(_entry.value());
			}
		}
	}

}