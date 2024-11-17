#include <Core/Context.h>

namespace MMPEngine::Core
{
	AppContext::AppContext(const AppContextSettings& baseSettings) :
		baseSettings(baseSettings),
		windowSize({0,0}),
#ifdef MMPENGINE_WIN
		platform(PlatformType::Win)
#endif
#ifdef MMPENGINE_MAC
		platform(PlatformType::Mac)
#endif
	{
	}
	AppContext::~AppContext() = default;
}