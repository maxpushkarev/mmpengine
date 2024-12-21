#include <stdexcept>
#include <Core/Pool.hpp>

namespace MMPEngine::Core
{
	Pool::Pool(const Settings& settings) : _settings(settings)
	{
	}
	Pool::~Pool() = default;

	Pool::Entry Pool::AllocateEntry()
	{
		auto newBlockCapacity = _settings.initialCapacity;

		for(std::size_t blockIndex = 0; blockIndex < _blocks.size(); ++blockIndex)
		{
			const auto slot = _blocks[blockIndex]->TryAllocateSlot();
			if(slot.has_value())
			{
				return {static_cast<std::uint32_t>(blockIndex), slot.value()};
			}

			newBlockCapacity *= _settings.growthFactor;
		}

		_blocks.emplace_back(InstantiateBlock(newBlockCapacity));
		return {static_cast<std::uint32_t>(_blocks.size() - 1), _blocks.back()->TryAllocateSlot().value()};
	}

	void Pool::ReleaseEntry(const Entry& entry)
	{
		_blocks.at(entry.blockIndex)->Release(entry.slotIndexInBlock);
	}

	Pool::Handle::Handle() = default;

	Pool::Handle::Handle(const std::shared_ptr<Pool>& pool, const Entry& entry) : _entry(entry), _pool(pool)
	{
	}

	Pool::Handle::Handle(Handle&& movableHandle) noexcept
		: _entry{ movableHandle._entry }, _pool{ std::move(movableHandle._pool) }
	{
		movableHandle._pool.reset();
		movableHandle._entry = std::nullopt;
	}

	Pool::Handle& Pool::Handle::operator=(Handle&& movableHandle) noexcept
	{
		if (this != &movableHandle)
		{
			_pool = std::move(movableHandle._pool);
			_entry = movableHandle._entry;

			movableHandle._pool.reset();
			movableHandle._entry = std::nullopt;
		}
		return *this;
	}

	Pool::Handle::~Handle()
	{
		if(_entry.has_value())
		{
			if (const auto poolStrongRef = _pool.lock())
			{
				poolStrongRef->ReleaseEntry(_entry.value());
			}
		}
	}

	Pool::Block::Block(std::uint32_t size) : _freeSlots(static_cast<decltype(_freeSlots)::size_type>(size), true)
	{
	}

	Pool::Block::~Block() = default;

	std::optional<std::uint32_t> Pool::Block::TryAllocateSlot()
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

	void Pool::Block::Release(std::uint32_t slotIndex)
	{
		if (_freeSlots.at(slotIndex))
		{
			throw std::logic_error("can not release free slot");
		}
		_freeSlots[slotIndex] = true;
	}
}