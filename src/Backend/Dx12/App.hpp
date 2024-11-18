#pragma once
#include <Core/App.hpp>
#include <Backend/Dx12/Context.hpp>

namespace MMPEngine::Backend::Dx12
{
	class RootApp final : public Core::RootApp<AppContext>
	{
	public:
		RootApp(const std::shared_ptr<AppContext>& context, const std::shared_ptr<Core::UserApp>& userApp);
		void Initialize() override;
	};
}
