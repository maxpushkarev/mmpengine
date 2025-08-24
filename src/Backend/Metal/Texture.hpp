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
        
        virtual MTL::PixelFormat GetFormat() const = 0;
        virtual NS::UInteger GetSamplesCount() const = 0;
        virtual MTL::Texture* GetNativeTexture() const = 0;
        
    protected:
        static NS::UInteger GetSamplesCount(Core::TargetTexture::Settings::Antialiasing aa);
    };

    class IDepthStencilTexture : public ITargetTexture
    {
    };

    class IColorTargetTexture : public ITargetTexture
    {
    };

    class BaseTexture : public ResourceEntity
    {
    protected:
        std::shared_ptr<Wrapper::Device> _device;
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
        
        MTL::TextureDescriptor* _nativeTextureDesc = nullptr;
        NS::SharedPtr<MTL::Texture> _nativeTexture = nullptr;
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
                void OnComplete(const std::shared_ptr<Core::BaseStream>& stream) override;
            };

        public:
            InitTask(const std::shared_ptr<InitTaskContext>& ctx);
        protected:
            void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
        };
    public:
        DepthStencilTargetTexture(const Settings& settings);
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
        MTL::PixelFormat GetFormat() const override;
        NS::UInteger GetSamplesCount() const override;
        MTL::Texture* GetNativeTexture() const override;
    };
}

