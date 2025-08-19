#pragma once
#include <filesystem>

namespace MMPEngine::Feature
{
    class Path final
    {
    public:
        static const std::filesystem::path& GetExecutablePath();
    private:
        static std::filesystem::path GetExecutablePathImpl();
    };
}
