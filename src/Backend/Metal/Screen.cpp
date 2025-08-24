#include <cassert>
#include <Backend/Metal/Screen.hpp>
#include <Core/Texture.hpp>
#include <Backend/Metal/Entity.hpp>
#include <Backend/Metal/Texture.hpp>


namespace MMPEngine::Backend::Metal
{
    Screen::Screen(const Settings& settings) : Core::Screen(settings)
    {
    }


    Screen::~Screen()
    {
        if(_screenPool)
        {
            _screenPool->release();
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
        return Core::BaseTask::kEmpty;
    }

    std::shared_ptr<Core::BaseTask> Screen::CreatePresentationTaskInternal()
    {
        const auto ctx = std::make_shared<ScreenTaskContext>();
        ctx->entity = std::dynamic_pointer_cast<Screen>(shared_from_this());

        return std::make_shared<Core::BatchTask>(std::initializer_list<std::shared_ptr<Core::BaseTask>> {
                Core::StreamFlushTask::kInstance,
                std::make_shared<PresentTask>(ctx)
            });
    }

    std::shared_ptr<Core::ColorTargetTexture> Screen::GetBackBuffer() const
    {
        return _backBuffer;
    }

    Screen::BackBuffer::BackBuffer(const Settings& settings)
        : Core::ColorTargetTexture(settings)
    {
       
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
        
        screen->_screenPool = NS::AutoreleasePool::alloc()->init();
        screen->_metalLayer = CA::MetalLayer::layer();
        
        screen->_backBuffer = std::make_shared<BackBuffer>(Core::ColorTargetTexture::Settings{
            Core::ColorTargetTexture::Settings::Format::R8G8B8A8_Float_01,
                screen->_settings.clearColor,
            { Core::TargetTexture::Settings::Antialiasing::MSAA_0, _specificGlobalContext->windowSize, "Screen::BackBuffer" }
            });
        
    }

    Screen::PresentTask::PresentTask(const std::shared_ptr<ScreenTaskContext>& ctx) : Task(ctx)
    {
    }

    void Screen::PresentTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);
        
        const auto screen = GetTaskContext()->entity;
    }

}

