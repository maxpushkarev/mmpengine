#include <Backend/Vulkan/Memory.hpp>
#include <cassert>
#include <optional>

namespace MMPEngine::Backend::Vulkan
{
	DeviceMemoryBlock::DeviceMemoryBlock(const Settings& settings) : _settings(settings), _mem(nullptr)
	{
	}

	DeviceMemoryBlock::~DeviceMemoryBlock()
	{
		if(_mem && _device)
		{
			vkFreeMemory(_device->GetNativeLogical(), _mem, nullptr);
		}
	}

	DeviceMemoryBlock::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task<MMPEngine::Backend::Vulkan::DeviceMemoryBlock::InitTaskContext>(ctx)
	{
	}

	void DeviceMemoryBlock::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		const auto entity = GetTaskContext()->entity;
		entity->_device = _specificGlobalContext->device;

		VkMemoryAllocateInfo info {};
		info.allocationSize = static_cast<VkDeviceSize>(entity->_settings.byteSize);
		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.pNext = nullptr;

		std::optional<std::uint32_t> memoryTypeIndex = std::nullopt;

		VkPhysicalDeviceMemoryProperties memProps {};
		vkGetPhysicalDeviceMemoryProperties(entity->_device->GetNativePhysical(), &memProps);

		for(std::uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
		{
			const auto& vkMemType = memProps.memoryTypes[i];
			if(vkMemType.propertyFlags & entity->_settings.flags)
			{
				memoryTypeIndex = i;
			}
		}

		assert(memoryTypeIndex.has_value());
		info.memoryTypeIndex = memoryTypeIndex.value();

		const auto res = vkAllocateMemory(entity->_device->GetNativeLogical(), &info, nullptr, &entity->_mem);
		assert(res == VkResult::VK_SUCCESS);
	}

	std::shared_ptr<Core::BaseTask> DeviceMemoryBlock::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<DeviceMemoryBlock>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

}
