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
			std::size_t setsCount = 32;
			std::size_t setsGrowth = 1;
		};
	private:
		class Pool final
		{
		public:
			Pool(const std::shared_ptr<Wrapper::Device>& device, VkDescriptorPool pool);
			~Pool();
			Pool(const Pool&) = delete;
			Pool(Pool&&) noexcept = delete;
			Pool& operator=(const Pool&) = delete;
			Pool& operator=(Pool&&) noexcept = delete;
			VkDescriptorPool GetPool() const;
		private:
			std::shared_ptr<Wrapper::Device> _device;
			VkDescriptorPool _pool;
		};

	public:

		class Allocation final
		{
			friend class DescriptorPool;
		private:
			Allocation(const std::shared_ptr<Wrapper::Device>& device, const std::shared_ptr<Pool>& pool, VkDescriptorSet set, VkDescriptorSetLayout setLayout);
		public:
			~Allocation();
			Allocation(const Allocation&) = delete;
			Allocation(Allocation&&) noexcept;
			Allocation& operator=(const Allocation&) = delete;
			Allocation& operator=(Allocation&&) noexcept;

			VkDescriptorSet GetDescriptorSet() const;
			VkDescriptorSetLayout GetDescriptorSetLayout() const;
		private:
			std::shared_ptr<Wrapper::Device> _device;
			std::shared_ptr<Pool> _pool;
			VkDescriptorSet _set = VK_NULL_HANDLE;
			VkDescriptorSetLayout _layout = VK_NULL_HANDLE;
		};

		DescriptorPool(const Settings&);
		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&&) noexcept = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;
		DescriptorPool& operator=(DescriptorPool&&) noexcept = delete;
		~DescriptorPool() override;

		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		Allocation AllocateSet(const VkDescriptorSetLayoutCreateInfo& layoutCreateInfo);

	private:

		void CreateNewPool(const VkDescriptorSetLayoutCreateInfo& layoutCreateInfo);

		std::shared_ptr<Wrapper::Device> _device;
		Settings _settings;
		std::vector<std::shared_ptr<Pool>> _pools;

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
