#include <Backend/Dx12/Memory.hpp>
#include <cassert>

namespace MMPEngine::Backend::Dx12
{
    DeviceMemoryBlock::DeviceMemoryBlock(const D3D12_HEAP_DESC& desc) : _desc(desc)
    {
    }

	Microsoft::WRL::ComPtr<ID3D12Heap> DeviceMemoryBlock::GetNative() const
    {
        return _nativeHeap;
    }

    DeviceMemoryBlock::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task<MMPEngine::Backend::Dx12::DeviceMemoryBlock::InitTaskContext>(ctx)
    {
    }

    void DeviceMemoryBlock::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);
        
        const auto entity = GetTaskContext()->entity;

		_specificGlobalContext->device->CreateHeap(
			&entity->_desc,
			IID_PPV_ARGS(&entity->_nativeHeap)
		);

		assert(entity->_nativeHeap);
    }

    std::shared_ptr<Core::BaseTask> DeviceMemoryBlock::CreateInitializationTask()
    {
        const auto ctx = std::make_shared<InitTaskContext>();
        ctx->entity = std::dynamic_pointer_cast<DeviceMemoryBlock>(shared_from_this());
        return std::make_shared<InitTask>(ctx);
    }

}

