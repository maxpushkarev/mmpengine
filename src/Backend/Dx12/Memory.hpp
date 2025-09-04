#pragma once
#include <Core/Entity.hpp>
#include <Backend/Dx12/Task.hpp>

namespace MMPEngine::Backend::Dx12
{
    class DeviceMemoryBlock final : public Core::BaseEntity
    {
    public:
        DeviceMemoryBlock(const D3D12_HEAP_DESC&);
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		Microsoft::WRL::ComPtr<ID3D12Heap> GetNative() const;
    private:
		D3D12_HEAP_DESC _desc;
        Microsoft::WRL::ComPtr<ID3D12Heap> _nativeHeap;
        
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

