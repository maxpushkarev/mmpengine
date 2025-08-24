#pragma once
#include <Core/Passkey.hpp>
#include <Core/Stream.hpp>
#include <Metal/Metal.hpp>

namespace MMPEngine::Backend::Metal
{
    class GlobalContext;

    namespace Wrapper
    {
        class Device final
        {
        public:
            Device();
            MTL::Device* GetNative() const;
        private:
            NS::SharedPtr<MTL::Device> _device = nullptr;
        };
    
        class Queue final
        {
        public:
            Queue(const std::shared_ptr<GlobalContext>&, std::uint32_t maxCmdBuffersCount);
            MTL::CommandQueue* GetNative() const;
        private:
            std::shared_ptr<GlobalContext> _globalContext;
            NS::SharedPtr<MTL::CommandQueue> _queue = nullptr;
        };
    
        class CommandBuffer final
        {
        public:
            
            using PassControl = Core::PassControl<true, Core::BaseStream>;
            
            CommandBuffer(const std::shared_ptr<GlobalContext>&, const std::shared_ptr<Queue>&);
            CommandBuffer(const CommandBuffer&) = delete;
            CommandBuffer(CommandBuffer&&) noexcept = delete;
            CommandBuffer& operator=(const CommandBuffer&) = delete;
            CommandBuffer& operator=(CommandBuffer&&) noexcept = delete;
            ~CommandBuffer();
            
            MTL::CommandBuffer* GetNative() const;
            void Reset(PassControl);
        private:
            std::shared_ptr<Queue> _queue;
            std::shared_ptr<GlobalContext> _globalContext;
            MTL::CommandBuffer* _commandBuffer = nullptr;
            MTL::CommandBufferDescriptor* _commandBufferDescriptor = nullptr;
        };
    
        class LogState final
        {
        public:
            LogState(const std::shared_ptr<Device>&, std::uint32_t bufferSize, MTL::LogLevel logLevel);
            MTL::LogState* GetNative() const;
        private:
            std::shared_ptr<Device> _device;
            NS::SharedPtr<MTL::LogState> _logState = nullptr;
        };
    }
}
