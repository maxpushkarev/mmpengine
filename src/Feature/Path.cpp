#include <Feature/Path.hpp>

#ifdef MMPENGINE_WIN
#include <windows.h>
#endif

#ifdef MMPENGINE_MAC
#include <mach-o/dyld.h>
#include <libgen.h>
#include <limits.h>
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
		char buffer[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, buffer);
		return std::filesystem::path{ buffer };
#endif
        return {};
    }
}
