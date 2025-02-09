#include <Backend/Dx12/Entity.hpp>
#include <Backend/Dx12/d3dx12.h>

namespace MMPEngine::Backend::Dx12
{
	BaseEntity::BaseEntity() = default;
	BaseEntity::~BaseEntity() = default;
	ResourceEntity::ResourceEntity() = default;

	const BaseDescriptorPool::Handle* BaseEntity::GetResourceDescriptorHandle() const
	{
		return nullptr;
	}

	const BaseDescriptorPool::Handle* BaseEntity::GetDSVDescriptorHandle() const
	{
		return nullptr;
	}

	const BaseDescriptorPool::Handle* BaseEntity::GetRTVDescriptorHandle() const
	{
		return nullptr;
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

	D3D12_GPU_VIRTUAL_ADDRESS ResourceEntity::GetNativeGPUAddressWithRequiredOffset() const
	{
		return _nativeResource->GetGPUVirtualAddress() + static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(_offsetInsideResource);
	}

	void ResourceEntity::SetNativeResource(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource, std::uint32_t offsetInsideResource)
	{
		_nativeResource = nativeResource;
		_offsetInsideResource = offsetInsideResource;
	}


	ResourceEntity::SwitchStateTask::SwitchStateTask(const std::shared_ptr<SwitchStateTaskContext>& context) : Task(context)
	{
	}

	void ResourceEntity::SwitchStateTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		if(const auto tc = GetTaskContext() ; const auto entity = tc->entity)
		{
			if( 
				((entity->_currentStateMask & tc->nextStateMask) != tc->nextStateMask) ||
				(tc->nextStateMask == 0 && entity->_currentStateMask != tc->nextStateMask)
			)
			{
				const D3D12_RESOURCE_BARRIER transitions[] = {
					CD3DX12_RESOURCE_BARRIER::Transition(entity->GetNativeResource().Get(),  entity->_currentStateMask, tc->nextStateMask)
				};

				_specificStreamContext->PopulateCommandsInBuffer()->ResourceBarrier(static_cast<std::uint32_t>(std::size(transitions)), transitions);
				entity->_currentStateMask = tc->nextStateMask;
			}
		}
	}
}
