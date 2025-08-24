#pragma once
#include <QuartzCore/QuartzCore.hpp>
#include <Metal/Metal.hpp>
#include <Core/Screen.hpp>
#include <Backend/Metal/Entity.hpp>
#include <Backend/Metal/Texture.hpp>

namespace MMPEngine::Backend::Metal
{
    class Screen final : public Core::Screen
    {
    private:
        class BackBuffer final : public Core::ColorTargetTexture, public Metal::BaseTexture, public IColorTargetTexture
        {
        public:
            BackBuffer(const Settings& settings, CA::MetalLayer* layer);
            MTL::PixelFormat GetFormat() const override;
            NS::UInteger GetSamplesCount() const override;
            void Next();
            CA::MetalDrawable* GetDrawable() const;
        protected:
            std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const override;
        private:
            CA::MetalLayer* _layer = nullptr;
            CA::MetalDrawable* _drawable = nullptr;
        };
        
        class ScreenTaskContext final : public Core::EntityTaskContext<Screen>
        {
        };

        class InitTask final : public Task<ScreenTaskContext>
        {
        public:
            InitTask(const std::shared_ptr<ScreenTaskContext>& ctx);
        protected:
            void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
        };

        class StartFrameTask final : public Task<ScreenTaskContext>
        {
        public:
            StartFrameTask(const std::shared_ptr<ScreenTaskContext>& ctx);
        protected:
            void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
        };
        
        class PresentTask final : public Task<ScreenTaskContext>
        {
        public:
            PresentTask(const std::shared_ptr<ScreenTaskContext>& ctx);
        protected:
            void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
        };
        
    public:
        Screen(const Settings& settings);
        Screen(const Screen&) = delete;
        Screen(Screen&&) noexcept = delete;
        Screen& operator=(const Screen&) = delete;
        Screen& operator=(Screen&&) noexcept = delete;
        ~Screen() override;
        std::shared_ptr<Core::ColorTargetTexture> GetBackBuffer() const override;
    protected:
        std::shared_ptr<Core::BaseTask> CreateInitializationTaskInternal() override;
        std::shared_ptr<Core::BaseTask> CreateStartFrameTaskInternal() override;
        std::shared_ptr<Core::BaseTask> CreatePresentationTaskInternal() override;
    private:
        std::shared_ptr<BackBuffer> _backBuffer;
        CA::MetalLayer* _metalLayer = nullptr;
    };
}
