#include <Backend/Dx12/Entity.hpp>
#include <Backend/Dx12/d3dx12.h>

namespace MMPEngine::Backend::Dx12
{

	ResourceEntity::ResourceEntity() = default;

	ResourceEntity::ResourceEntity(std::string_view name) : Core::BaseEntity(name)
	{
	}

	void ResourceEntity::SetUpNativeResourceName()
	{
		const auto str = GetName();
		std::wstring wName(std::begin(str), std::end(str));
		_nativeResource->SetName(wName.c_str());
	}

	std::shared_ptr<Core::BaseTask> ResourceEntity::CreateSwitchStateTask(D3D12_RESOURCE_STATES nextStateMask)
	{
		const auto switchTaskContext = std::make_shared<SwitchStateTaskContext>();
		switchTaskContext->entity = std::dynamic_pointer_cast<ResourceEntity>(shared_from_this());
		switchTaskContext->nextStateMask = nextStateMask;
		return std::make_shared<SwitchStateTask>(switchTaskContext);
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> ResourceEntity::GetNativeResource() const
	{
		return _nativeResource;
	}

	void ResourceEntity::SetNativeResource(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource)
	{
		_nativeResource = nativeResource;
		SetUpNativeResourceName();
	}


	ResourceEntity::SwitchStateTask::SwitchStateTask(const std::shared_ptr<SwitchStateTaskContext>& context) : TaskWithInternalContext(context)
	{
	}

	void ResourceEntity::SwitchStateTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		if(const auto entity = _internalTaskContext->entity.lock())
		{
			if((entity->_currentStateMask & _internalTaskContext->nextStateMask) != _internalTaskContext->nextStateMask)
			{
				const D3D12_RESOURCE_BARRIER transitions[] = {
					CD3DX12_RESOURCE_BARRIER::Transition(entity->GetNativeResource().Get(),  entity->_currentStateMask, _internalTaskContext->nextStateMask)
				};

				if(const auto sc = _specificStreamContext.lock())
				{
					sc->cmdList->ResourceBarrier(static_cast<std::uint32_t>(std::size(transitions)), transitions);
				}

				entity->_currentStateMask = _internalTaskContext->nextStateMask;
			}
		}
	}

	void ResourceEntity::SwitchStateTask::Finalize(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Finalize(stream);
	}
}
