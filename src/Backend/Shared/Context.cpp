#include <Backend/Shared/Context.hpp>

namespace MMPEngine::Backend::Shared
{
    bool BaseStreamContext::IsCommandsPopulated(PassControl) const
    {
        return _commandsPopulated;
    }

    bool BaseStreamContext::IsCommandsClosed(PassControl) const
    {
        return _commandsClosed;
    }

    void BaseStreamContext::SetCommandsPopulated(PassControl, bool value)
    {
        _commandsPopulated = value;
    }

    void BaseStreamContext::SetCommandsClosed(PassControl, bool value)
    {
        _commandsClosed = value;
    }
}
