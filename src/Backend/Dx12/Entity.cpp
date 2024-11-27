#include <Backend/Dx12/Entity.hpp>
#include <Backend/Dx12/d3dx12.h>

namespace MMPEngine::Backend::Dx12
{
	BaseEntity::BaseEntity(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource) : _nativeResource(nativeResource)
	{
		SetUpNativeResourceName();
	}

	BaseEntity::BaseEntity(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource, std::string_view name) : Core::BaseEntity(name), _nativeResource(nativeResource)
	{
		SetUpNativeResourceName();
	}

	void BaseEntity::SetUpNativeResourceName()
	{
		const auto str = GetName();
		std::wstring wName(std::begin(str), std::end(str));
		_nativeResource->SetName(wName.c_str());
	}

	std::shared_ptr<Core::BaseTask> BaseEntity::CreateSwitchStateTask(D3D12_RESOURCE_STATES nextStateMask)
	{
		return std::make_shared<SwitchStateTask>(std::make_shared<SwitchStateTaskContext>(shared_from_this(), nextStateMask));
	}


	BaseEntity::SwitchStateTaskContext::SwitchStateTaskContext(const std::shared_ptr<BaseEntity>& entity, D3D12_RESOURCE_STATES nextState)
		: nextStateMask(nextState), entity(entity)
	{
	}

	BaseEntity::SwitchStateTask::SwitchStateTask(const std::shared_ptr<SwitchStateTaskContext>& context) : TaskWithInnerContext(context)
	{
	}

	void BaseEntity::SwitchStateTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		if(const auto entity = _innerContext->entity.lock())
		{
			if((entity->_currentStateMask & _innerContext->nextStateMask) != _innerContext->nextStateMask)
			{
				const D3D12_RESOURCE_BARRIER transitions[] = {
					CD3DX12_RESOURCE_BARRIER::Transition(entity->_nativeResource.Get(),  entity->_currentStateMask, _innerContext->nextStateMask)
				};

				if(const auto sc = _specificStreamContext.lock())
				{
					sc->cmdList->ResourceBarrier(static_cast<std::uint32_t>(std::size(transitions)), transitions);
				}

				entity->_currentStateMask = _innerContext->nextStateMask;
			}
		}
	}

	void BaseEntity::SwitchStateTask::Finalize(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Finalize(stream);
	}
}
