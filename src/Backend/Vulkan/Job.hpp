#pragma once
#include <vulkan/vulkan.h>
#include <Core/Material.hpp>
#include <Backend/Vulkan/Context.hpp>
#include <Backend/Vulkan/Task.hpp>
#include <Backend/Vulkan/Descriptor.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class BaseJob
	{
	public:
		BaseJob(const BaseJob&) = delete;
		BaseJob(BaseJob&&) noexcept = delete;
		BaseJob& operator=(const BaseJob&) = delete;
		BaseJob& operator=(BaseJob&&) noexcept = delete;

	protected:
		BaseJob();
		virtual	~BaseJob();

		void PrepareMaterialParameters(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params);
		virtual VkShaderStageFlags GetStageFlags() const = 0;

		std::vector<std::shared_ptr<Core::BaseTask>> _memoryBarrierTasks;
		std::vector<DescriptorPool::Allocation> _setAllocations;
		std::vector<VkDescriptorSet> _sets;
		VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
		VkPipeline _pipeline = VK_NULL_HANDLE;
		std::shared_ptr<Wrapper::Device> _device;

		class TaskContext final : public Core::TaskContext
		{
		public:
			std::shared_ptr<BaseJob> job;
		};

		class MemBarriersTask final : public Task<TaskContext>
		{
		public:
			MemBarriersTask(const std::shared_ptr<TaskContext>& context);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	};

	template<typename TCoreMaterial>
	class Job : public BaseJob
	{
		static_assert(std::is_base_of_v<Core::BaseMaterial, TCoreMaterial>, "TCoreMaterial must be derived from Core::BaseMaterial");
	protected:
		VkShaderStageFlags GetStageFlags() const override;
	};

	template<>
	VkShaderStageFlags Job<Core::ComputeMaterial>::GetStageFlags() const;
	template<>
	VkShaderStageFlags Job<Core::MeshMaterial>::GetStageFlags() const;

	template<typename TCoreMaterial>
	inline VkShaderStageFlags Job<TCoreMaterial>::GetStageFlags() const
	{
		return VK_SHADER_STAGE_ALL;
	}


}