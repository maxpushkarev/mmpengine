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

		Device::Device(const std::shared_ptr<Instance>& instance, VkPhysicalDevice physicalDevice, VkDevice device) : _instance(instance), _physicalDevice(physicalDevice), _logicalDevice(device)
		{
		}

		Device::~Device()
		{
			vkDeviceWaitIdle(_logicalDevice);
			vkDestroyDevice(_logicalDevice, nullptr);
		}

		VkDevice Device::GetNativeLogical() const
		{
			return _logicalDevice;
		}

		VkPhysicalDevice Device::GetNativePhysical() const
		{
			return _physicalDevice;
		}

		CommandAllocator::CommandAllocator(const std::shared_ptr<Device>& device, VkCommandPool pool)
			: _device(device), _pool(pool)
		{
		}

		CommandAllocator::~CommandAllocator()
		{
			vkDestroyCommandPool(_device->GetNativeLogical(), _pool, nullptr);
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
			vkFreeCommandBuffers(_device->GetNativeLogical(), _allocator->GetNative(), 1, &_buffer);
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

		Fence::Fence(const std::shared_ptr<Device>& device, VkFence fence) : _device(device), _fence(fence)
		{
		}

		Fence::~Fence()
		{
			vkDestroyFence(_device->GetNativeLogical(), _fence, nullptr);
		}

		VkFence Fence::GetNative() const
		{
			return _fence;
		}
	}
}