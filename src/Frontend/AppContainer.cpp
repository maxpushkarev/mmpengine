#include <Frontend/AppContainer.hpp>

namespace MMPEngine::Frontend
{
	AppContainer::AppContainer(Settings&& settings, const std::shared_ptr<Core::BaseRootApp>& app) : _settings(std::move(settings)), _app(app)
	{
		_inputController = std::make_shared<Core::AppInputController>(app);
	}

	AppContainer::~AppContainer() = default;

#ifdef MMPENGINE_WIN
	namespace Win
	{
		AppContainer::AppContainer(PlatformAppContainer::Settings&& settings, const std::shared_ptr<Core::BaseRootApp>& app) :
			PlatformAppContainer<MMPEngine::Frontend::Win::AppContainerSetting>(std::move(settings), app)
		{
			
		}


	}
#endif

}
