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

	StreamContext::StreamContext(const std::shared_ptr<GlobalContext>& globalContext, VkCommandPool commandPool, VkCommandBuffer commandBuffer) :
		_globalContext(globalContext), _commandPool(commandPool), _commandBuffer(commandBuffer)
	{
	}

	StreamContext::~StreamContext()
	{
		if(_commandBuffer)
		{
			vkFreeCommandBuffers(_globalContext->device, _commandPool, 1, &_commandBuffer);
		}

		if(_commandPool)
		{
			vkDestroyCommandPool(_globalContext->device, _commandPool, nullptr);
		}
	}
}