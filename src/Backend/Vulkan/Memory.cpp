#include <Backend/Vulkan/Memory.hpp>

namespace MMPEngine::Backend::Vulkan
{
	DeviceMemoryBlock::DeviceMemoryBlock(const Settings& settings) : _settings(settings)
	{
	}

	DeviceMemoryBlock::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task<MMPEngine::Backend::Vulkan::DeviceMemoryBlock::InitTaskContext>(ctx)
	{
	}

	void DeviceMemoryBlock::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
	}

	std::shared_ptr<Core::BaseTask> DeviceMemoryBlock::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<DeviceMemoryBlock>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

}