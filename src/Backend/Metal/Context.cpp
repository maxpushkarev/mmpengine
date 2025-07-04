#include <Backend/Metal/Context.hpp>

namespace MMPEngine::Backend::Metal
{
    GlobalContext::GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m) : Core::GlobalContext(s, std::move(m))
    {
    }
}
