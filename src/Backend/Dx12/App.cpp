#include <Backend/Dx12/App.hpp>

namespace MMPEngine::Backend::Dx12
{
	RootApp::RootApp(const std::shared_ptr<AppContext>& context, const std::shared_ptr<Core::UserApp>& userApp) : Core::RootApp(context, userApp)
	{
	}
}