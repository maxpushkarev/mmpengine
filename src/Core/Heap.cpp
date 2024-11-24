#include <Core/Heap.hpp>

namespace MMPEngine::Core
{
	BaseHeap::BaseHeap(const Settings& settings) : _settings(settings)
	{
	}
	BaseHeap::~BaseHeap() = default;

	BaseHeap::Handle::Handle(const std::shared_ptr<BaseHeap>& heap, const Entry& entry) : _entry(entry), _heap(heap)
	{
	}

	BaseHeap::Handle::Handle(Handle&& movableHandle) noexcept
		: _entry{ movableHandle._entry }, _heap{ std::move(movableHandle._heap) }
	{
		movableHandle._heap.reset();
		movableHandle._entry = std::nullopt;
	}

	BaseHeap::Handle& BaseHeap::Handle::operator=(Handle&& movableHandle) noexcept
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

	BaseHeap::Handle::~Handle()
	{
		if(_entry.has_value())
		{
			if (const auto heapStrongRef = _heap.lock())
			{
				heapStrongRef->Release(_entry.value());
			}
		}
	}
}