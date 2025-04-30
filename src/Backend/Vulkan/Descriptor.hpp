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
			struct Entry
			{
				VkDescriptorPoolSize initialSizeInfo;
				std::size_t growth = 1;
			};
			std::vector<Entry> entries;
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
		std::vector<VkDescriptorPool> _pools;

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
