#include <Backend/Dx12/Entity.hpp>
#include <Backend/Dx12/d3dx12.h>

namespace MMPEngine::Backend::Dx12
{
	BaseEntity::BaseEntity() = default;
	BaseEntity::~BaseEntity() = default;


	ResourceEntity::ResourceEntity() = default;

	ResourceEntity::ResourceEntity(std::string_view name)
	{
	}

	const BaseDescriptorHeap::Handle* BaseEntity::GetShaderInVisibleDescriptorHandle() const
	{
		return nullptr;
	}

	const BaseDescriptorHeap::Handle* BaseEntity::GetShaderVisibleDescriptorHandle() const
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

	D3D12_GPU_VIRTUAL_ADDRESS ResourceEntity::GetNativeGPUAddress() const
	{
		return _nativeResource->GetGPUVirtualAddress() + static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(_offsetInsideResource);
	}

	void ResourceEntity::SetNativeResource(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource, std::uint32_t offsetInsideResource)
	{
		_nativeResource = nativeResource;
		_offsetInsideResource = offsetInsideResource;
	}


	ResourceEntity::SwitchStateTask::SwitchStateTask(const std::shared_ptr<SwitchStateTaskContext>& context) : ContextualTask(context)
	{
	}

	void ResourceEntity::SwitchStateTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
	}

	void ResourceEntity::SwitchStateTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		if(const auto entity = _taskContext->entity)
		{
			if((entity->_currentStateMask & _taskContext->nextStateMask) != _taskContext->nextStateMask)
			{
				const D3D12_RESOURCE_BARRIER transitions[] = {
					CD3DX12_RESOURCE_BARRIER::Transition(entity->GetNativeResource().Get(),  entity->_currentStateMask, _taskContext->nextStateMask)
				};

				_specificStreamContext->PopulateCommandsInList()->ResourceBarrier(static_cast<std::uint32_t>(std::size(transitions)), transitions);
				entity->_currentStateMask = _taskContext->nextStateMask;
			}
		}
	}

	void ResourceEntity::SwitchStateTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}
}
