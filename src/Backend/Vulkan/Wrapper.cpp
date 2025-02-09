#include <Backend/Vulkan/Wrapper.hpp>

namespace MMPEngine::Backend::Vulkan
{
	namespace Wrapper
	{
		Instance::Instance(VkInstance instance) : _instance(instance)
		{
		}

		Instance::~Instance()
		{
			vkDestroyInstance(_instance, nullptr);
		}

		Device::Device(const std::shared_ptr<Instance>& instance, VkDevice device) : _instance(instance), _device(device)
		{
		}

		Device::~Device()
		{
			vkDeviceWaitIdle(_device);
			vkDestroyDevice(_device, nullptr);
		}

		VkDevice Device::GetNative() const
		{
			return _device;
		}

		CommandAllocator::CommandAllocator(const std::shared_ptr<Device>& device, VkCommandPool pool)
			: _device(device), _pool(pool)
		{
		}

		CommandAllocator::~CommandAllocator()
		{
			vkDestroyCommandPool(_device->GetNative(), _pool, nullptr);
		}

		VkCommandPool CommandAllocator::GetNative() const
		{
			return _pool;
		}

		CommandBuffer::CommandBuffer(
			const std::shared_ptr<Device>& device,
			const std::shared_ptr<CommandAllocator>& allocator, 
			VkCommandBuffer buffer
		)
			: _device(device), _allocator(allocator), _buffer(buffer)
		{
		}

		CommandBuffer::~CommandBuffer()
		{
			vkFreeCommandBuffers(_device->GetNative(), _allocator->GetNative(), 1, &_buffer);
		}

		VkCommandBuffer CommandBuffer::GetNative() const
		{
			return _buffer;
		}

		Queue::Queue(VkQueue queue, std::uint32_t familyIndex) : _queue(queue), _familyIndex(familyIndex)
		{
		}

		std::uint32_t Queue::GetFamilyIndex() const
		{
			return _familyIndex;
		}

		VkQueue Queue::GetNative() const
		{
			return _queue;
		}

	}
}