#pragma once
#include <vulkan/vulkan.h>
#include <Core/Material.hpp>
#include <Backend/Vulkan/Context.hpp>
#include <Backend/Vulkan/Task.hpp>
#include <Backend/Vulkan/Entity.hpp>
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

		void BakeMaterialParameters(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params);
		virtual VkShaderStageFlags GetStageFlags() const = 0;

		std::vector<std::shared_ptr<Core::BaseTask>> _switchMaterialParametersStateTasks;
		std::vector<std::function<void(const std::shared_ptr<StreamContext>& streamContext)>> _applyMaterialParametersCallbacks;
		std::vector<DescriptorPool::Allocation> _setAllocations;

		class TaskContext final : public Core::TaskContext
		{
		public:
			std::shared_ptr<BaseJob> job;
		};

		class ApplyParametersTask : public Task<TaskContext>
		{
		private:
			class SwitchState final : public Task<TaskContext>
			{
			public:
				SwitchState(const std::shared_ptr<TaskContext>& context);
				void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

			class Apply final : public Task<TaskContext>
			{
			public:
				Apply(const std::shared_ptr<TaskContext>& context);
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

		public:
			ApplyParametersTask(const std::shared_ptr<TaskContext>& context);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;

		private:
			std::shared_ptr<Core::BaseTask> _switchState;
			std::shared_ptr<Core::BaseTask> _apply;
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

	template<typename TCoreMaterial>
	inline VkShaderStageFlags Job<TCoreMaterial>::GetStageFlags() const
	{
		return VK_SHADER_STAGE_ALL;
	}


}