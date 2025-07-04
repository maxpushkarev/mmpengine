
#include <Backend/Metal/Stream.hpp>

namespace MMPEngine::Backend::Metal
{
    Stream::Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext)
        : Core::BaseStream(globalContext, streamContext)
    {
    }
}
