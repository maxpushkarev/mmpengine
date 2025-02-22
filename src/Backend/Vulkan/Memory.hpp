#pragma once
#include <optional>
#include <Core/Entity.hpp>
#include <Backend/Vulkan/Task.hpp>
#include <Backend/Vulkan/Wrapper.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class DeviceMemoryBlock final : public Core::BaseEntity
	{
	public:
		struct Settings final
		{
			std::size_t byteSize;
			VkMemoryPropertyFlagBits includeFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM;
			VkMemoryPropertyFlagBits excludeFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM;
		};

		DeviceMemoryBlock(const Settings&);
		DeviceMemoryBlock(const DeviceMemoryBlock&) = delete;
		DeviceMemoryBlock(DeviceMemoryBlock&&) noexcept = delete;
		DeviceMemoryBlock& operator=(const DeviceMemoryBlock&) = delete;
		DeviceMemoryBlock& operator=(DeviceMemoryBlock&&) noexcept = delete;
		~DeviceMemoryBlock() override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		static std::optional<std::uint32_t> FindMemoryType(VkPhysicalDevice physicalDevice, VkMemoryPropertyFlagBits includeFlags, VkMemoryPropertyFlagBits excludeFlags);
		VkDeviceMemory GetNative() const;
		void* GetHost() const;
	private:
		Settings _settings;
		std::shared_ptr<Wrapper::Device> _device;
		VkDeviceMemory _deviceMem;
		void* _hostMem = nullptr;

		class InitTaskContext final : public Core::EntityTaskContext<DeviceMemoryBlock>
		{
		};

		class InitTask final : public Task<InitTaskContext>
		{
		public:
			InitTask(const std::shared_ptr<InitTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	};
}
