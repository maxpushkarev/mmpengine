#include <Backend/Metal/Memory.hpp>
#include <cassert>

namespace MMPEngine::Backend::Metal
{
    DeviceMemoryBlock::DeviceMemoryBlock(const Settings& settings) : _settings(settings)
    {
    }

    DeviceMemoryBlock::~DeviceMemoryBlock()
    {
        if(_nativeHeap)
        {
            _nativeHeap->release();
        }
    }

    MTL::Heap* DeviceMemoryBlock::GetNative() const
    {
        return _nativeHeap;
    }

    DeviceMemoryBlock::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task<MMPEngine::Backend::Metal::DeviceMemoryBlock::InitTaskContext>(ctx)
    {
    }

    void DeviceMemoryBlock::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);
        
        const auto entity = GetTaskContext()->entity;
        entity->_device = _specificGlobalContext->device;

        const auto desc = MTL::HeapDescriptor::alloc()->init();
        
        desc->setSize(static_cast<NS::UInteger>(entity->_settings.size));
        desc->setType(entity->_settings.type);
        desc->setStorageMode(entity->_settings.storageMode);
        desc->setResourceOptions(entity->_settings.resourceOption);
        desc->setCpuCacheMode(entity->_settings.cpuCacheMode);
        desc->setSparsePageSize(entity->_settings.sparsePageSize);
        desc->setHazardTrackingMode(MTL::HazardTrackingModeTracked);
        
        entity->_nativeHeap = entity->_device->GetNative()->newHeap(desc);
        desc->release();
    }

    std::shared_ptr<Core::BaseTask> DeviceMemoryBlock::CreateInitializationTask()
    {
        const auto ctx = std::make_shared<InitTaskContext>();
        ctx->entity = std::dynamic_pointer_cast<DeviceMemoryBlock>(shared_from_this());
        return std::make_shared<InitTask>(ctx);
    }

}

