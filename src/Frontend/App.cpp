#include <Frontend/App.hpp>
#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/App.hpp>
#endif

namespace MMPEngine::Frontend
{
	std::shared_ptr<Core::App> App::BuildRootApp(const Core::AppContextSettings& appContextSettings, const std::shared_ptr<Core::UserApp>& userApp)
	{
		if(appContextSettings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			const auto rootApp = std::make_shared<Backend::Dx12::RootApp>(std::make_shared<Backend::Dx12::AppContext>(appContextSettings));
			rootApp->Attach(userApp);
			return rootApp;
#else
			throw Core::UnsupportedException("unable to create root app for DX12 backend");
#endif
		}

		throw std::runtime_error("unable to create root app");
	}

}