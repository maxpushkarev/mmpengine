#pragma once
#include <Core/Context.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class GlobalContext : public Core::GlobalContext
	{
	public:
		GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m);
	};

	class StreamContext : public Core::StreamContext
	{
	public:
		StreamContext();
	};
}