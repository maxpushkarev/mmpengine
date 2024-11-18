#include <Backend/Dx12/App.hpp>

namespace MMPEngine::Backend::Dx12
{
	RootApp::RootApp(const std::shared_ptr<AppContext>& context)
		: Core::RootApp<AppContext>(context)
	{
	}

	void RootApp::Initialize()
	{
		//init dx12 context data
		Core::RootApp<AppContext>::Initialize();
	}
}