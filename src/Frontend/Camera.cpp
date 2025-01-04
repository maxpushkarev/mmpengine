#include <Frontend/Camera.hpp>

namespace MMPEngine::Frontend
{
	PerspectiveCamera::PerspectiveCamera(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings, const std::shared_ptr<Core::Node>& node)
		: Camera<MMPEngine::Core::PerspectiveCamera>(globalContext, settings, node)
	{
	}

	OrthographicCamera::OrthographicCamera(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings, const std::shared_ptr<Core::Node>& node)
		: Camera<MMPEngine::Core::OrthographicCamera>(globalContext, settings, node)
	{
	}

}