#include <stdexcept>
#include <Core/Heap.hpp>

namespace MMPEngine::Core
{
	BaseItemHeap::BaseItemHeap(const Settings& settings) : _settings(settings)
	{
	}
	BaseItemHeap::~BaseItemHeap() = default;

	BaseItemHeap::Entry BaseItemHeap::AllocateEntry()
	{
		auto newBlockCapacity = _settings.initialCapacity;

		for(std::size_t blockIndex = 0; blockIndex < _blocks.size(); ++blockIndex)
		{
			const auto slot = _blocks[blockIndex]->TryAllocate();
			if(slot.has_value())
			{
				return {static_cast<std::uint32_t>(blockIndex), slot.value()};
			}

			newBlockCapacity *= _settings.growthFactor;
		}

		_blocks.emplace_back(InstantiateBlock(newBlockCapacity));
		return {static_cast<std::uint32_t>(_blocks.size() - 1), _blocks.back()->TryAllocate().value()};
	}

	void BaseItemHeap::ReleaseEntry(const Entry& entry)
	{
		_blocks.at(entry.blockIndex)->Release(entry.slotIndexInBlock);
	}

	BaseItemHeap::Handle::Handle() = default;

	BaseItemHeap::Handle::Handle(const std::shared_ptr<BaseItemHeap>& heap, const Entry& entry) : _entry(entry), _heap(heap)
	{
	}

	BaseItemHeap::Handle::Handle(Handle&& movableHandle) noexcept
		: _entry{ movableHandle._entry }, _heap{ std::move(movableHandle._heap) }
	{
		movableHandle._heap.reset();
		movableHandle._entry = std::nullopt;
	}

	BaseItemHeap::Handle& BaseItemHeap::Handle::operator=(Handle&& movableHandle) noexcept
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

	BaseItemHeap::Handle::~Handle()
	{
		if(_entry.has_value())
		{
			if (const auto heapStrongRef = _heap.lock())
			{
				heapStrongRef->ReleaseEntry(_entry.value());
			}
		}
	}

	BaseItemHeap::Block::Block(std::uint32_t size) : _freeSlots(static_cast<decltype(_freeSlots)::size_type>(size), true)
	{
	}

	BaseItemHeap::Block::~Block() = default;

	std::optional<std::uint32_t> BaseItemHeap::Block::TryAllocate()
	{
		const auto freeSlot = std::find_if(_freeSlots.begin(), _freeSlots.end(), [](const bool& slot)
		{
			return slot;
		});

		if(freeSlot == _freeSlots.cend())
		{
			return std::nullopt;
		}

		*freeSlot = false;
		return static_cast<std::uint32_t>(std::distance(_freeSlots.begin(), freeSlot));
	}

	void BaseItemHeap::Block::Release(std::uint32_t slotIndex)
	{
		if (_freeSlots.at(slotIndex))
		{
			throw std::logic_error("can not release free slot");
		}
		_freeSlots[slotIndex] = true;
	}
}