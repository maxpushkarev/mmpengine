#include <Backend/Vulkan/Descriptor.hpp>

namespace MMPEngine::Backend::Vulkan
{
	DescriptorPool::DescriptorPool(const Settings& settings) : _settings(settings)
	{
	}

	DescriptorPool::~DescriptorPool()
	{
		if (_device)
		{
			for (auto& pool : _pools)
			{
				vkDestroyDescriptorPool(_device->GetNativeLogical(), pool, nullptr);
			}
		}
	}

	void DescriptorPool::CreateNativePool(const VkDescriptorSetLayoutCreateInfo& layoutCreateInfo)
	{
		std::vector<VkDescriptorPoolSize> poolSizes (_settings.entries.size(), VkDescriptorPoolSize {});
		for (std::size_t i = 0; i < _settings.entries.size(); ++i)
		{
			const auto& entry = _settings.entries[i];
			auto& ps = poolSizes[i];

			ps = entry.initialSizeInfo;
			ps.descriptorCount = 0;

			for (std::size_t j = 0; j < static_cast<std::size_t>(layoutCreateInfo.bindingCount); ++j)
			{
				const auto& binding = layoutCreateInfo.pBindings[j];

				if (ps.type == binding.descriptorType)
				{
					ps.descriptorCount = binding.descriptorCount;
				}
			}
		}

		for (std::size_t i = 0; i < poolSizes.size(); ++i)
		{
			const auto& entry = _settings.entries[i];
			auto& ps = poolSizes[i];

			std::size_t m = (std::max)(static_cast<std::size_t>(1), entry.growth * _pools.size());
			ps.descriptorCount = (std::max)(ps.descriptorCount,
				static_cast<std::uint32_t>(m) * entry.initialSizeInfo.descriptorCount
			);
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<std::uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 0;
		poolInfo.pNext = nullptr;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_ALLOW_OVERALLOCATION_SETS_BIT_NV;

		VkDescriptorPool pool;
		vkCreateDescriptorPool(_device->GetNativeLogical(), &poolInfo, nullptr, &pool);
		_pools.push_back(pool);
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
}