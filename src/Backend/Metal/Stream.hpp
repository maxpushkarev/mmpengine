#pragma once
#include <Core/Stream.hpp>
#include <Backend/Metal/Context.hpp>
#include <Backend/Shared/Stream.hpp>

namespace MMPEngine::Backend::Metal
{
    class Stream : public Core::BaseStream
    {
    public:
        Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext);
    protected:
        void RestartInternal() override;
        void SyncInternal() override;
        void SubmitInternal() override;
        void Flush() override;
    private:
        std::shared_ptr<StreamContext> _specificStreamContext;
    };
}
