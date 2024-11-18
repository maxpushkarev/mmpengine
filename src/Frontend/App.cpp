#include <Frontend/App.hpp>
#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/App.hpp>
#endif

namespace MMPEngine::Frontend
{
	std::shared_ptr<Core::App> App::BuildRootApp(const std::shared_ptr<Core::UserApp>& userApp)
	{
		const auto appContext = userApp->GetContext();
		if(appContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
		return std::make_shared<Backend::Dx12::RootApp>(std::dynamic_pointer_cast<Backend::Dx12::AppContext>(appContext), userApp);
#else
		throw Core::UnsupportedException("unable to create root app for DX12 backend");
#endif
		}

		throw std::runtime_error("unable to create root app");
	}

}