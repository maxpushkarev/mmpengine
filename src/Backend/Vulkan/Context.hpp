#pragma once
#include <Core/Context.hpp>
#include <vulkan/vulkan.h>

namespace MMPEngine::Backend::Vulkan
{
	class GlobalContext : public Core::GlobalContext
	{
	public:
		GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m);
		GlobalContext(const GlobalContext&) = delete;
		GlobalContext(GlobalContext&&) noexcept = delete;
		GlobalContext& operator=(const GlobalContext&) = delete;
		GlobalContext& operator=(GlobalContext&&) noexcept = delete;
		~GlobalContext() override;
	public:
		VkInstance instance;
		VkDevice device;
	};

	class StreamContext : public Core::StreamContext
	{
	public:
		StreamContext();
	};
}