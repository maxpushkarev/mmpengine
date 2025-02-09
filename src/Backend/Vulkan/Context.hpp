#pragma once
#include <Core/Context.hpp>
#include <Backend/Vulkan/Wrapper.hpp>
#include <vulkan/vulkan.h>

namespace MMPEngine::Backend::Vulkan
{
	class GlobalContext : public Core::GlobalContext
	{
	public:
		GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m);
	public:
		std::shared_ptr<Wrapper::Instance> instance;
		std::shared_ptr<Wrapper::Device> device;
	};


	class StreamContext : public Core::StreamContext
	{
	public:
	private:
	};
}