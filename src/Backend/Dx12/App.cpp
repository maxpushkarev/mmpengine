#include <Backend/Dx12/App.hpp>

namespace MMPEngine::Backend::Dx12
{
	RootApp::RootApp(const std::shared_ptr<AppContext>& context, const std::shared_ptr<Core::UserApp>& userApp)
		: Core::RootApp<AppContext>(context, userApp)
	{
	}

	void RootApp::Initialize()
	{
		//init dx12 context data
		Core::RootApp<AppContext>::Initialize();
	}
}