#include <Backend/Vulkan/Context.hpp>

namespace MMPEngine::Backend::Vulkan
{
	GlobalContext::GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m) : Core::GlobalContext(s, std::move(m)),
		instance(nullptr), device(nullptr)
	{
	}

	GlobalContext::~GlobalContext()
	{
		if(device)
		{
			vkDeviceWaitIdle(device);
			vkDestroyDevice(device, nullptr);
		}

		if(instance)
		{
			vkDestroyInstance(instance, nullptr);
		}
	}

	StreamContext::StreamContext() = default;
}