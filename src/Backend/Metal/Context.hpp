#pragma once
#include <Core/Context.hpp>
#include <Backend/Metal/Wrapper.hpp>

namespace MMPEngine::Backend::Metal
{
    class GlobalContext : public Core::GlobalContext
    {
    public:
        GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m);
        std::shared_ptr<Wrapper::Device> device;
    };

    class StreamContext : public Core::StreamContext
    {
        
    };
}
