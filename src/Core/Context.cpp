#include <Core/Context.hpp>

namespace MMPEngine::Core
{
	Context::Context() = default;
	Context::~Context() = default;

	Context::CustomProperties::CustomProperties() = default;
	Context::CustomProperties::~CustomProperties() = default;

	AppContext::AppContext(const AppContextSettings& s) :
		settings(s),
		windowSize({0,0}),
#ifdef MMPENGINE_WIN
		platform(PlatformType::Win)
#endif
#ifdef MMPENGINE_MAC
		platform(PlatformType::Mac)
#endif
	{
		input = std::make_unique<Input>();
	}
}