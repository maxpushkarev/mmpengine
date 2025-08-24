#include <cassert>
#include <Backend/Metal/Screen.hpp>
#include <Core/Texture.hpp>
#include <Backend/Metal/Entity.hpp>
#include <Backend/Metal/Texture.hpp>
#include <Backend/Metal/Window.h>


namespace MMPEngine::Backend::Metal
{
    Screen::Screen(const Settings& settings) : Core::Screen(settings)
    {
    }


    Screen::~Screen()
    {
        if(_metalLayer)
        {
            _metalLayer->release();
        }
    }


    std::shared_ptr<Core::BaseTask> Screen::CreateInitializationTaskInternal()
    {
        const auto ctx = std::make_shared<ScreenTaskContext>();
        ctx->entity = std::dynamic_pointer_cast<Screen>(shared_from_this());

        return std::make_shared<InitTask>(ctx);
    }

    std::shared_ptr<Core::BaseTask> Screen::CreateStartFrameTaskInternal()
    {
        const auto ctx = std::make_shared<ScreenTaskContext>();
        ctx->entity = std::dynamic_pointer_cast<Screen>(shared_from_this());

        return std::make_shared<StartFrameTask>(ctx);
    }

    std::shared_ptr<Core::BaseTask> Screen::CreatePresentationTaskInternal()
    {
        const auto ctx = std::make_shared<ScreenTaskContext>();
        ctx->entity = std::dynamic_pointer_cast<Screen>(shared_from_this());

        return std::make_shared<PresentTask>(ctx);
    }

    std::shared_ptr<Core::ColorTargetTexture> Screen::GetBackBuffer() const
    {
        return _backBuffer;
    }

    Screen::BackBuffer::BackBuffer(const Settings& settings, CA::MetalLayer* layer)
        : Core::ColorTargetTexture(settings), _layer(layer)
    {
       
    }

    void Screen::BackBuffer::Next()
    {
        _drawable = _layer->nextDrawable();
    }

    CA::MetalDrawable* Screen::BackBuffer::GetDrawable() const
    {
        return _drawable;
    }

    std::shared_ptr<DeviceMemoryHeap> Screen::BackBuffer::GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const
    {
        return nullptr;
    }

    MTL::PixelFormat Screen::BackBuffer::GetFormat() const
    {
        //TODO
        return MTL::PixelFormatInvalid;
    }

    NS::UInteger Screen::BackBuffer::GetSamplesCount() const
    {
        return 1U;
    }

    Screen::InitTask::InitTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task(ctx)
    {
    }

    void Screen::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);

        const auto screen = GetTaskContext()->entity;
        
        auto pool = NS::AutoreleasePool::alloc()->init();
        
        screen->_metalLayer = CA::MetalLayer::layer();
        screen->_metalLayer->retain();
        
        const auto format = screen->_settings.gammaCorrection ? MTL::PixelFormatBGRA8Unorm_sRGB : MTL::PixelFormatBGRA8Unorm;
        
        screen->_metalLayer->setDevice(_specificGlobalContext->device->GetNative());
        screen->_metalLayer->setPixelFormat(format);
        screen->_metalLayer->setFramebufferOnly(true);
        screen->_metalLayer->setDrawableSize({
            static_cast<CGFloat>(_specificGlobalContext->windowSize.x),
            static_cast<CGFloat>(_specificGlobalContext->windowSize.y)
        });
        

        Window::JoinMetalLayerToWindow(_specificGlobalContext->nativeWindow, screen->_metalLayer);
        
        screen->_backBuffer = std::make_shared<BackBuffer>(Core::ColorTargetTexture::Settings{
            Core::ColorTargetTexture::Settings::Format::R8G8B8A8_Float_01,
                screen->_settings.clearColor,
            { Core::TargetTexture::Settings::Antialiasing::MSAA_0, _specificGlobalContext->windowSize, "Screen::BackBuffer" }
            }, screen->_metalLayer);
        
        pool->release();
    }

    Screen::StartFrameTask::StartFrameTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task(ctx)
    {
    }

    void Screen::StartFrameTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);
        
        const auto screen = GetTaskContext()->entity;
        screen->_backBuffer->Next();
    }

    Screen::PresentTask::PresentTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task(ctx)
    {
    }

    void Screen::PresentTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);
        
        const auto screen = GetTaskContext()->entity;
        const auto drawable = screen->_backBuffer->GetDrawable();
        if(drawable)
        {
            _specificStreamContext->PopulateCommandsInBuffer()->GetNative()->presentDrawable(drawable);
        }
    }
}

