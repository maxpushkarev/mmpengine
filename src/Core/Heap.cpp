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
	}
	Heap::Block::~Block() = default;


	std::optional<Heap::Block::Range> Heap::Block::TryAllocate(const Request& request)
	{

		return std::nullopt;
	}

	void Heap::Block::Release(const Range& range)
	{
	}

	Heap::Entry Heap::Allocate(const Request& request)
	{
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
				assert((range.value().to - range.value().from + 1) == request.size);

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