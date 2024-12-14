#include <Core/Context.hpp>

namespace MMPEngine::Core
{
	Context::Context() = default;
	Context::~Context() = default;

	Context::Properties::Properties() = default;
	Context::Properties::~Properties() = default;

	AppContext::AppContext(const Settings& s, std::unique_ptr<Core::Math>&& m, std::unique_ptr<Core::BaseLogger>&& l) :
		settings(s)
		,windowSize({0,0})
#ifdef MMPENGINE_WIN
		,platform(PlatformType::Win)
#endif
#ifdef MMPENGINE_MAC
		,platform(PlatformType::Mac)
#endif
		,math(std::move(m))
		,logger(std::move(l))
		,input(std::make_unique<Input>())
#ifdef MMPENGINE_WIN
		,nativeWindow(nullptr)
#endif
	{
	}
}