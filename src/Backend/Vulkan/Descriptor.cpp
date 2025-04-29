#include <Backend/Vulkan/Descriptor.hpp>

namespace MMPEngine::Backend::Vulkan
{
	DescriptorPool::DescriptorPool(const Settings& settings) : _settings(settings), _pool(nullptr)
	{
	}

	DescriptorPool::~DescriptorPool()
	{
		if (_device && _pool)
		{
			vkDestroyDescriptorPool(_device->GetNativeLogical(), _pool, nullptr);
		}
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