#include <Backend/Vulkan/Job.hpp>
#include <Core/Buffer.hpp>

namespace MMPEngine::Backend::Vulkan
{
	BaseJob::BaseJob() = default;
	BaseJob::~BaseJob() = default;

	void BaseJob::BakeMaterialParameters(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params)
	{
		const auto& paramsVec = params.GetAll();

		std::uint32_t bindingCounter = 0;
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		for (std::size_t i = 0; i < paramsVec.size(); ++i)
		{
			if (i > 0 && paramsVec[i - 1].tag != paramsVec[i].tag)
			{
				VkDescriptorSetLayoutCreateInfo layoutInfo{};
				layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layoutInfo.bindingCount = static_cast<std::uint32_t>(bindings.size());
				layoutInfo.pBindings = bindings.data();

				auto setAllocation = globalContext->descriptorPool->AllocateSet(layoutInfo);
				_setAllocations.push_back(std::move(setAllocation));

				bindingCounter = 0;
				bindings.clear();
				continue;
			}

			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = bindingCounter++;
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = GetStageFlags();
			layoutBinding.pImmutableSamplers = nullptr;

			if (std::holds_alternative<Core::BaseMaterial::Parameters::Buffer>(paramsVec[i].settings))
			{
				if (std::dynamic_pointer_cast<const Core::BaseUniformBuffer>(paramsVec[i].entity))
				{
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				}
				else
				{
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				}
			}
			else
			{
				throw Core::UnsupportedException("unsupported entity type as parameter for Vulkan backend");
			}
			bindings.push_back(layoutBinding);
		}

		if (!bindings.empty())
		{
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<std::uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();

			auto setAllocation = globalContext->descriptorPool->AllocateSet(layoutInfo);
			_setAllocations.push_back(std::move(setAllocation));
		}
	}


	BaseJob::ApplyParametersTask::ApplyParametersTask(const std::shared_ptr<TaskContext>& context) : Task<TaskContext>(context)
	{
		_switchState = std::make_shared<SwitchState>(context);
		_apply = std::make_shared<Apply>(context);
	}

	void BaseJob::ApplyParametersTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		stream->Schedule(_switchState);
		stream->Schedule(_apply);
	}

	BaseJob::ApplyParametersTask::SwitchState::SwitchState(const std::shared_ptr<TaskContext>& context) : Task<TaskContext>(context)
	{
	}

	void BaseJob::ApplyParametersTask::SwitchState::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		if (const auto job = this->GetTaskContext()->job)
		{
			for (const auto& sst : job->_switchMaterialParametersStateTasks)
			{
				stream->Schedule(sst);
			}
		}
	}

	BaseJob::ApplyParametersTask::Apply::Apply(const std::shared_ptr<TaskContext>& context) : Task<TaskContext>(context)
	{
	}

	void BaseJob::ApplyParametersTask::Apply::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		if (const auto job = this->GetTaskContext()->job; const auto sc = _specificStreamContext)
		{
			for (const auto& applyParameterCallback : job->_applyMaterialParametersCallbacks)
			{
				applyParameterCallback(sc);
			}
		}
	}

	template<>
	VkShaderStageFlags Job<Core::ComputeMaterial>::GetStageFlags() const
	{
		return VK_SHADER_STAGE_COMPUTE_BIT;
	}
}
