#pragma once
#include <Core/Entity.hpp>
#include <Backend/Metal/Task.hpp>
#include <Backend/Metal/Wrapper.hpp>
#include <Metal/Metal.hpp>

namespace MMPEngine::Backend::Metal
{
    class DeviceMemoryBlock final : public Core::BaseEntity
    {
    public:
        struct Settings final
        {
            std::uint32_t size = 0;
            MTL::HeapType type = MTL::HeapTypePlacement;
            MTL::StorageMode storageMode = MTL::StorageModePrivate;
            MTL::ResourceOptions resourceOption = MTL::ResourceOptionCPUCacheModeDefault;
            MTL::CPUCacheMode cpuCacheMode = MTL::CPUCacheModeDefaultCache;
            MTL::SparsePageSize sparsePageSize = MTL::SparsePageSize16;
        };

        DeviceMemoryBlock(const Settings&);
        DeviceMemoryBlock(const DeviceMemoryBlock&) = delete;
        DeviceMemoryBlock(DeviceMemoryBlock&&) noexcept = delete;
        DeviceMemoryBlock& operator=(const DeviceMemoryBlock&) = delete;
        DeviceMemoryBlock& operator=(DeviceMemoryBlock&&) noexcept = delete;
        ~DeviceMemoryBlock() override;
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
        MTL::Heap* GetNative() const;
    private:
        Settings _settings;
        std::shared_ptr<Wrapper::Device> _device;
        MTL::Heap* _nativeHeap = nullptr;
        
        class InitTaskContext final : public Core::EntityTaskContext<DeviceMemoryBlock>
        {
        };

        class InitTask final : public Task<InitTaskContext>
        {
        public:
            InitTask(const std::shared_ptr<InitTaskContext>& ctx);
        protected:
            void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
        };
    };
}

