#pragma once
#include <Core/App.hpp>
#include <Backend/Dx12/Context.hpp>

namespace MMPEngine::Backend::Dx12
{
	class RootApp final : public Core::RootApp<GlobalContext>
	{
	public:
		RootApp(const std::shared_ptr<GlobalContext>& context);
		void Initialize() override;
	};
}
