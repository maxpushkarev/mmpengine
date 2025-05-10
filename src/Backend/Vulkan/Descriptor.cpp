#include <Backend/Vulkan/Descriptor.hpp>
#include <cassert>

namespace MMPEngine::Backend::Vulkan
{
	DescriptorPool::Pool::Pool(const std::shared_ptr<Wrapper::Device>& device, VkDescriptorPool pool) : _device(device), _pool(pool)
	{
	}

	DescriptorPool::Pool::~Pool()
	{
		if (_device && _pool)
		{
			vkDestroyDescriptorPool(_device->GetNativeLogical(), _pool, nullptr);
		}
	}

	VkDescriptorPool DescriptorPool::Pool::GetPool() const
	{
		return _pool;
	}

	DescriptorPool::DescriptorPool(const Settings& settings) : _settings(settings)
	{
	}

	DescriptorPool::~DescriptorPool() = default;

	void DescriptorPool::CreateNewPool(const VkDescriptorSetLayoutCreateInfo& layoutCreateInfo)
	{
		std::vector<VkDescriptorPoolSize> poolSizes (_settings.entries.size(), VkDescriptorPoolSize {});
		std::uint32_t maxSets = 1;

		for (std::size_t i = 0; i < _settings.entries.size(); ++i)
		{
			const auto& entry = _settings.entries[i];
			auto& ps = poolSizes[i];

			ps = entry.initialSizeInfo;

			for (std::size_t j = 0; j < static_cast<std::size_t>(layoutCreateInfo.bindingCount); ++j)
			{
				const auto& binding = layoutCreateInfo.pBindings[j];

				if (ps.type == binding.descriptorType)
				{
					ps.descriptorCount = binding.descriptorCount;
				}
			}

			std::size_t m = (std::max)(static_cast<std::size_t>(1), entry.growth * _pools.size());
			ps.descriptorCount = (std::max)(ps.descriptorCount,
				static_cast<std::uint32_t>(m) * entry.initialSizeInfo.descriptorCount);

			maxSets = (std::max)(1U,static_cast<std::uint32_t>(
				(std::max)(static_cast<std::size_t>(1), _settings.setsGrowth * _pools.size())
			) * static_cast<std::uint32_t>(_settings.setsCount));
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<std::uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = maxSets;
		poolInfo.pNext = nullptr;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		VkDescriptorPool pool;
		vkCreateDescriptorPool(_device->GetNativeLogical(), &poolInfo, nullptr, &pool);
		_pools.push_back(std::make_shared<Pool>(_device, pool));
	}

	DescriptorPool::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task<MMPEngine::Backend::Vulkan::DescriptorPool::InitTaskContext>(ctx)
	{
	}

	void DescriptorPool::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		const auto entity = GetTaskContext()->entity;
		entity->_device = _specificGlobalContext->device;
	}

	std::shared_ptr<Core::BaseTask> DescriptorPool::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<DescriptorPool>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	DescriptorPool::Allocation DescriptorPool::AllocateSet(const VkDescriptorSetLayoutCreateInfo& layoutCreateInfo)
	{
		VkDescriptorSetLayout layout = nullptr;
		VkDescriptorSet set = nullptr;
		std::shared_ptr<Pool> pool = nullptr;

		vkCreateDescriptorSetLayout(_device->GetNativeLogical(), &layoutCreateInfo, nullptr, &layout);
		assert(layout != nullptr);

		for (const auto& p : _pools)
		{
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = p->GetPool();
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &layout;
			allocInfo.pNext = nullptr;

			const auto res = vkAllocateDescriptorSets(_device->GetNativeLogical(), &allocInfo, &set);

			if (res == VK_SUCCESS)
			{
				pool = p;
				break;
			}

			assert(res == VK_ERROR_OUT_OF_POOL_MEMORY || res == VK_ERROR_FRAGMENTED_POOL);
		}

		if (!set)
		{
			CreateNewPool(layoutCreateInfo);

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = _pools.back()->GetPool();
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &layout;
			allocInfo.pNext = nullptr;

			const auto res = vkAllocateDescriptorSets(_device->GetNativeLogical(), &allocInfo, &set);
			pool = _pools.back();
			assert(res == VK_SUCCESS);
		}

		assert(set != nullptr);
		return Allocation {_device, pool, set, layout};
	}

	DescriptorPool::Allocation::Allocation(const std::shared_ptr<Wrapper::Device>& device, const std::shared_ptr<Pool>& pool, VkDescriptorSet set, VkDescriptorSetLayout setLayout)
		: _device(device), _pool(pool), _set(set), _layout(setLayout)
	{
		
	}

	DescriptorPool::Allocation::~Allocation()
	{
		if (_set && _pool && _device)
		{
			vkFreeDescriptorSets(_device->GetNativeLogical(), _pool->GetPool(), 1, &_set);
		}

		if (_layout && _device)
		{
			vkDestroyDescriptorSetLayout(_device->GetNativeLogical(), _layout, nullptr);
		}
	}

	DescriptorPool::Allocation::Allocation(Allocation&& other) noexcept
		: _device(std::move(other._device)), _pool(std::move(other._pool)), _set(other._set), _layout(other._layout)
	{
		other._device = nullptr;
		other._pool = nullptr;
		other._set = nullptr;
		other._layout = nullptr;
	}

	DescriptorPool::Allocation& DescriptorPool::Allocation::operator=(Allocation&& other) noexcept
	{
		if (this != &other)
		{
			_device = other._device;
			_pool = other._pool;
			_set = other._set;
			_layout = other._layout;

			other._device = nullptr;
			other._pool = nullptr;
			other._set = nullptr;
			other._layout = nullptr;
		}

		return *this;
	}

	VkDescriptorSet DescriptorPool::Allocation::GetDescriptorSet() const
	{
		return _set;
	}

	VkDescriptorSetLayout DescriptorPool::Allocation::GetDescriptorSetLayout() const
	{
		return _layout;
	}

}