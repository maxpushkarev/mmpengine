#pragma once
#include <Core/Stream.hpp>
#include <Backend/Vulkan/Context.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class Stream : public Core::Stream<GlobalContext, StreamContext>
	{
	private:
		using Super = Core::Stream<GlobalContext, StreamContext>;
	public:
		Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext);
	};
}