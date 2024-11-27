#pragma once
#include <Core/Task.hpp>
#include <Backend/Dx12/Context.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Task : public Core::OuterContextSpecificTask<AppContext, StreamContext>
	{
	protected:
		Task();
	};
}
