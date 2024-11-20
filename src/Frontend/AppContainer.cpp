#include <Frontend/AppContainer.hpp>

namespace MMPEngine::Frontend
{
	AppContainer::AppContainer(const std::shared_ptr<Core::BaseRootApp>& app) : _app(app)
	{
		_inputController = std::make_shared<Core::AppInputController>(app);
	}

	AppContainer::~AppContainer() = default;
}
