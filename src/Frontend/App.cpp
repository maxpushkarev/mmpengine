#include <Frontend/App.hpp>
#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/App.hpp>
#include <Backend/Dx12/Math.hpp>
#endif

namespace MMPEngine::Frontend
{
	std::shared_ptr<Core::BaseRootApp> App::BuildRootApp(
		const Core::AppContext::Settings& appContextSettings, 
		const std::shared_ptr<Core::UserApp>& userApp,
		std::unique_ptr<Core::Math>&& math,
		std::unique_ptr<Core::BaseLogger>&& logger)
	{
		if(appContextSettings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			if(!math)
			{
				math = std::make_unique<Backend::Dx12::DxMath>();
			}
			const auto rootApp = std::make_shared<Backend::Dx12::RootApp>(std::make_shared<Backend::Dx12::AppContext>(appContextSettings, std::move(math), std::move(logger)));
			rootApp->Attach(userApp);
			return rootApp;
#else
			throw Core::UnsupportedException("unable to create root app for DX12 backend");
#endif
		}

		throw std::runtime_error("unable to create root app");
	}

}