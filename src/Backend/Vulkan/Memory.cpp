#include <Backend/Vulkan/Memory.hpp>
#include <cassert>
#include <optional>

namespace MMPEngine::Backend::Vulkan
{
	DeviceMemoryBlock::DeviceMemoryBlock(const Settings& settings) : _settings(settings), _deviceMem(VK_NULL_HANDLE)
	{
	}

	DeviceMemoryBlock::~DeviceMemoryBlock()
	{
		if(_hostMem && _device)
		{
			vkUnmapMemory(_device->GetNativeLogical(), _deviceMem);
		}

		if(_deviceMem && _device)
		{
			vkFreeMemory(_device->GetNativeLogical(), _deviceMem, nullptr);
		}
	}

	VkDeviceMemory DeviceMemoryBlock::GetNative() const
	{
		return _deviceMem;
	}

	void* DeviceMemoryBlock::GetHost() const
	{
		return _hostMem;
	}

	std::optional<std::uint32_t> DeviceMemoryBlock::FindMemoryType(VkPhysicalDevice physicalDevice, VkMemoryPropertyFlagBits includeFlags, VkMemoryPropertyFlagBits excludeFlags)
	{
		VkPhysicalDeviceMemoryProperties memProps{};
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

		for (std::uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
		{
			const auto& vkMemType = memProps.memoryTypes[i];
			if (
				(memProps.memoryHeaps[vkMemType.heapIndex].flags == VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) &&
				((vkMemType.propertyFlags & includeFlags) == includeFlags) &&
				(((~vkMemType.propertyFlags) & excludeFlags) == excludeFlags)
				)
			{
				return i;
			}
		}

		return std::nullopt;
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

		const auto memoryTypeIndex = FindMemoryType(entity->_device->GetNativePhysical(), entity->_settings.includeFlags, entity->_settings.excludeFlags);
		assert(memoryTypeIndex.has_value());
		info.memoryTypeIndex = memoryTypeIndex.value();

		const auto res = vkAllocateMemory(entity->_device->GetNativeLogical(), &info, nullptr, &entity->_deviceMem);
		assert(res == VkResult::VK_SUCCESS);

		if((entity->_settings.includeFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
		{
			const auto res = vkMapMemory(
				entity->_device->GetNativeLogical(),
				entity->_deviceMem,
				0,
				static_cast<VkDeviceSize>(entity->_settings.byteSize),
				0,
				&entity->_hostMem
			);
			assert(res == VK_SUCCESS);
		}
	}

	std::shared_ptr<Core::BaseTask> DeviceMemoryBlock::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<DeviceMemoryBlock>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

}
