#pragma once
#include <Core/Heap.hpp>
#include <Backend/Vulkan/Wrapper.hpp>
#include <Backend/Vulkan/Task.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class DescriptorPool final : public Core::BaseEntity
	{
	public:
		struct Settings final
		{
			std::vector<VkDescriptorPoolSize> sizeInfos;
		};

		DescriptorPool(const Settings&);
		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&&) noexcept = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;
		DescriptorPool& operator=(DescriptorPool&&) noexcept = delete;
		~DescriptorPool() override;

		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;

	private:
		std::shared_ptr<Wrapper::Device> _device;
		Settings _settings;
		VkDescriptorPool _pool;

		class InitTaskContext final : public Core::EntityTaskContext<DescriptorPool>
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
