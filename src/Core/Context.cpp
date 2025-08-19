#include <Core/Context.hpp>

namespace MMPEngine::Core
{
	Context::Context() = default;
	Context::~Context() = default;

	Context::Properties::Properties() = default;
	Context::Properties::~Properties() = default;

	GlobalContext::GlobalContext(const Settings& s, const Environment& environment, std::unique_ptr<Core::Math>&& m) :
		settings(s)
        ,environment(environment)
		,windowSize({0,0})
#ifdef MMPENGINE_WIN
		,platform(PlatformType::Win)
#endif
#ifdef MMPENGINE_MAC
		,platform(PlatformType::Mac)
#endif
		,math(std::move(m))
#ifdef MMPENGINE_WIN
		,nativeWindow(nullptr)
#endif
	{
	}
}
