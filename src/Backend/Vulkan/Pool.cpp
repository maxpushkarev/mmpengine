#include <Backend/Vulkan/Pool.hpp>
#include <cassert>

namespace MMPEngine::Backend::Vulkan
{
	DescriptorPool::DescriptorPool(const std::shared_ptr<Wrapper::Device>& device, const Settings& settings)
		: Core::Pool(settings.base),
		_nativeSettings(settings.native), _device(device)
	{
	}

	std::unique_ptr<Core::Pool::Block> DescriptorPool::InstantiateBlock(std::uint32_t size)
	{
		return std::make_unique<Block>(_device, size, _nativeSettings);
	}

	DescriptorPool::Handle DescriptorPool::Allocate()
	{
		return { std::dynamic_pointer_cast<DescriptorPool>(shared_from_this()), AllocateEntry() };
	}

	DescriptorPool::Block::Block(const std::shared_ptr<Wrapper::Device>& device, std::uint32_t size, const NativeSettings& nativeSettings) : Core::Pool::Block(size), _device(device)
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = nativeSettings.type;
		poolSize.descriptorCount = size;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = 0;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_ALLOW_OVERALLOCATION_SETS_BIT_NV;
		poolInfo.pNext = nullptr;

		const auto res = vkCreateDescriptorPool(device->GetNativeLogical(), &poolInfo, nullptr, &_nativeDescPool);
		assert(res == VK_SUCCESS);
	}

	DescriptorPool::Block::~Block()
	{
		if (_nativeDescPool)
		{
			vkDestroyDescriptorPool(_device->GetNativeLogical(), _nativeDescPool, nullptr);
		}
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