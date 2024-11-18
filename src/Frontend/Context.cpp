#include <Frontend/Context.hpp>
#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Context.hpp>
#endif

namespace MMPEngine::Frontend
{
	std::shared_ptr<Core::AppContext> AppContext::BuildAppContext(const Core::AppContextSettings& appContextSettings)
	{
		if (appContextSettings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::AppContext>(appContextSettings);
#else
			throw Core::UnsupportedException("unable to create app context for DX12 backend");
#endif
		}

		throw std::runtime_error("unable to create app context");
	}

}