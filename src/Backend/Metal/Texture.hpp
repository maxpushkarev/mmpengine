#pragma once
#include <Core/Texture.hpp>
#include <Backend/Metal/Entity.hpp>

namespace MMPEngine::Backend::Metal
{
    class ITargetTexture
    {
    public:
        ITargetTexture();
        ITargetTexture(const ITargetTexture&) = delete;
        ITargetTexture(ITargetTexture&&) noexcept = delete;
        ITargetTexture& operator=(const ITargetTexture&) = delete;
        ITargetTexture& operator=(ITargetTexture&&) noexcept = delete;
        virtual ~ITargetTexture();
    };

    class IDepthStencilTexture : public ITargetTexture
    {
    };

    class IColorTargetTexture : public ITargetTexture
    {
    };

    class BaseTexture : public ResourceEntity
    {
    };

    class ResourceTexture : public BaseTexture
    {
    public:
        ResourceTexture();
        ~ResourceTexture() override;
        ResourceTexture(const ResourceTexture&) = delete;
        ResourceTexture(ResourceTexture&&) noexcept = delete;
        ResourceTexture& operator=(const ResourceTexture&) = delete;
        ResourceTexture& operator=(ResourceTexture&&) noexcept = delete;
    protected:

        std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
        
        MTL::Texture* _nativeTexture = nullptr;
    };

    class DepthStencilTargetTexture final : public Core::DepthStencilTargetTexture, public ResourceTexture, public IDepthStencilTexture
    {
    private:
        class InitTaskContext final : public Core::EntityTaskContext<DepthStencilTargetTexture>
        {
        };
        class InitTask final : public Task<InitTaskContext>
        {
        private:
            class Allocate final : public Task<InitTaskContext>
            {
            public:
                Allocate(const std::shared_ptr<InitTaskContext>& context);
                void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
            };

            class Create final : public Task<InitTaskContext>
            {
            public:
                Create(const std::shared_ptr<InitTaskContext>& context);
                void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
            };

        public:
            InitTask(const std::shared_ptr<InitTaskContext>& ctx);
        protected:
            void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
        };
    public:
        DepthStencilTargetTexture(const Settings& settings);
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
    };
}

