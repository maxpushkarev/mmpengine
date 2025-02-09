#pragma once
#include <memory>
#include <vulkan/vulkan.h>

namespace MMPEngine::Backend::Vulkan
{
	namespace Wrapper
	{
		class Instance final
		{
			public:
				Instance(VkInstance instance);
				~Instance();
				Instance(const Instance&) = delete;
				Instance(Instance&&) noexcept = delete;
				Instance& operator=(const Instance&) = delete;
				Instance& operator=(Instance&&) noexcept = delete;
			private:
				VkInstance _instance;
		};

		class Device final
		{
		public:
			Device(const std::shared_ptr<Instance>& instance, VkDevice device);
			Device(const Device&) = delete;
			Device(Device&&) noexcept = delete;
			Device& operator=(const Device&) = delete;
			Device& operator=(Device&&) noexcept = delete;
			~Device();
			VkDevice GetNative() const;
		private:
			std::shared_ptr<Instance> _instance;
			VkDevice _device;
		};

		class CommandAllocator final
		{
		public:
			CommandAllocator(const std::shared_ptr<Device>& device, VkCommandPool pool);
			CommandAllocator(const CommandAllocator&) = delete;
			CommandAllocator(CommandAllocator&&) = delete;
			CommandAllocator& operator=(const CommandAllocator&) = delete;
			CommandAllocator& operator=(CommandAllocator&&) = delete;
			~CommandAllocator();

			VkCommandPool GetNative() const;
		private:
			std::shared_ptr<Device> _device;
			VkCommandPool _pool;
		};

		class CommandBuffer final
		{
		public:
			CommandBuffer(
				const std::shared_ptr<Device>& device,
				const std::shared_ptr<CommandAllocator>& allocator, 
				VkCommandBuffer buffer
			);
			CommandBuffer(const CommandBuffer&) = delete;
			CommandBuffer(CommandBuffer&&) = delete;
			CommandBuffer& operator=(const CommandBuffer&) = delete;
			CommandBuffer& operator=(CommandBuffer&&) = delete;
			~CommandBuffer();

			VkCommandBuffer GetNative() const;
		private:
			std::shared_ptr<Device> _device;
			std::shared_ptr<CommandAllocator> _allocator;
			VkCommandBuffer _buffer;
		};

		class Queue final
		{
		public:
			Queue(VkQueue queue, std::uint32_t familyIndex);
			VkQueue GetNative() const;
			std::uint32_t GetFamilyIndex() const;
		private:
			VkQueue _queue;
			std::uint32_t _familyIndex;
		};

		class Fence final
		{
		public:
			Fence(const std::shared_ptr<Device>& device, VkFence fence);
			Fence(const Fence&) = delete;
			Fence(Fence&&) noexcept = delete;
			Fence& operator=(const Fence&) = delete;
			Fence& operator=(Fence&&) noexcept = delete;
			~Fence();
		private:
			std::shared_ptr<Device> _device;
			VkFence _fence;
		};
	}
}