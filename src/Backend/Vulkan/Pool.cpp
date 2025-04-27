#include <Backend/Vulkan/Pool.hpp>

namespace MMPEngine::Backend::Vulkan
{
	DescriptorPool::DescriptorPool(const Settings& settings)
		: Core::Pool(settings.base),
		_nativeSettings(settings.native)
	{
	}

	std::unique_ptr<Core::Pool::Block> DescriptorPool::InstantiateBlock(std::uint32_t size)
	{
		return std::make_unique<Block>(size, _nativeSettings);
	}

	DescriptorPool::Handle DescriptorPool::Allocate()
	{
		return { std::dynamic_pointer_cast<DescriptorPool>(shared_from_this()), AllocateEntry() };
	}

	DescriptorPool::Block::Block(std::uint32_t size, const NativeSettings& nativeSettings) : Core::Pool::Block(size)
	{
	}

	DescriptorPool::Handle::Handle() = default;
	
	DescriptorPool::Handle::Handle(const std::shared_ptr<DescriptorPool>& descHeap, const Entry& entry)
		: Core::Pool::Handle(descHeap, entry), _descHeap(descHeap)
	{
		if (_entry.has_value())
		{
		}
	}

}