#include "Material.hpp"

#include <Backend/Dx12/Material.hpp>

namespace MMPEngine::Backend::Dx12
{
	Material::Material() = default;
	Material::~Material() = default;

	void Material::ApplyParameters(const std::shared_ptr<StreamContext>& streamContext)
	{
		for (const auto& fn : _applyParametersCallbacks)
		{
			fn(streamContext);
		}
	}

	void Material::SwitchParametersStates(const std::shared_ptr<Core::BaseStream>& stream)
	{
		for (const auto& task : _switchStateTasks)
		{
			stream->Schedule(task);
		}
	}


	Material::ApplyParametersTask::ApplyParametersTask(const std::shared_ptr<ApplyMaterialTaskContext>& context) : Core::ContextualTask<ApplyMaterialTaskContext>(context)
	{
		_switchState = std::make_shared<SwitchState>(context);
		_apply = std::make_shared<Apply>(context);
	}

	void Material::ApplyParametersTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		stream->Schedule(_switchState);
		stream->Schedule(_apply);
	}

	void Material::ApplyParametersTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
	}

	void Material::ApplyParametersTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

	Material::ApplyParametersTask::SwitchState::SwitchState(const std::shared_ptr<ApplyMaterialTaskContext>& context) : Core::ContextualTask<ApplyMaterialTaskContext>(context)
	{
	}

	void Material::ApplyParametersTask::SwitchState::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		if (const auto mat = this->_internalTaskContext->materialPtr.lock())
		{
			mat->SwitchParametersStates(stream);
		}
	}

	void Material::ApplyParametersTask::SwitchState::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
	}

	void Material::ApplyParametersTask::SwitchState::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}


	Material::ApplyParametersTask::Apply::Apply(const std::shared_ptr<ApplyMaterialTaskContext>& context) : Core::ContextualTask<ApplyMaterialTaskContext>(context)
	{
	}

	void Material::ApplyParametersTask::Apply::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
	}

	void Material::ApplyParametersTask::Apply::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		if (const auto mat = this->_internalTaskContext->materialPtr.lock(); const auto sc = _specificStreamContext.lock())
		{
			mat->ApplyParameters(sc);
		}
	}

	void Material::ApplyParametersTask::Apply::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}


	ComputeMaterial::ComputeMaterial(const std::shared_ptr<Core::ComputeShader>& computeShader) : Core::ComputeMaterial(computeShader)
	{
	}

	std::shared_ptr<Core::BaseTask> ComputeMaterial::CreateTaskForApply()
	{
		const auto ctx = std::make_shared<ApplyMaterialTaskContext>();
		ctx->materialPtr = std::dynamic_pointer_cast<MaterialImpl<Core::ComputeMaterial>>(shared_from_this());
		return std::make_shared<ApplyParametersTask>(ctx);
	}

	std::shared_ptr<Core::BaseTask> ComputeMaterial::CreateTaskForUpdateParametersInternal()
	{
		const auto ctx = std::make_shared<ParametersUpdatedTaskContext>();
		ctx->materialImplPtr = std::dynamic_pointer_cast<MaterialImpl<Core::ComputeMaterial>>(shared_from_this());
		ctx->params = &_params;
		return std::make_shared<ParametersUpdatedTask>(ctx);
	}

}