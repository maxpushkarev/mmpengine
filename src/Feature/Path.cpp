#include <Feature/Path.hpp>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#ifdef MMPENGINE_WIN

#endif

#ifdef MMPENGINE_MAC
#include <mach-o/dyld.h>
#include <libgen.h>
#endif

namespace MMPEngine::Feature
{
    const std::filesystem::path& Path::GetExecutablePath()
    {
        static auto res = GetExecutablePathImpl();
        return res;
    }
        
    std::filesystem::path Path::GetExecutablePathImpl()
    {
#ifdef MMPENGINE_MAC
        char exePath[PATH_MAX];
        uint32_t size = sizeof(exePath);
        _NSGetExecutablePath(exePath, &size);
        
        char exeDir[PATH_MAX];
        strcpy(exeDir, exePath);
        return std::filesystem::path { dirname(exeDir) };
#endif
       
#ifdef MMPENGINE_WIN
        return {};
#endif
        return {};
    }
}
