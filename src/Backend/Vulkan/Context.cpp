#include <Backend/Vulkan/Context.hpp>

namespace MMPEngine::Backend::Vulkan
{
	GlobalContext::GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m) : Core::GlobalContext(s, std::move(m)),
		instance(VK_NULL_HANDLE), device(VK_NULL_HANDLE)
	{
	}

	StreamContext::StreamContext(
		const std::shared_ptr<Wrapper::Queue>& queue, 
		const std::shared_ptr<Wrapper::CommandAllocator>& allocator, 
		const std::shared_ptr<Wrapper::CommandBuffer>& cmdBuffer,
		const std::shared_ptr<Wrapper::Fence>& fence)
			: Shared::StreamContext<
				std::shared_ptr<MMPEngine::Backend::Vulkan::Wrapper::Queue>,
				std::shared_ptr<MMPEngine::Backend::Vulkan::Wrapper::CommandAllocator>,
				std::shared_ptr<MMPEngine::Backend::Vulkan::Wrapper::CommandBuffer>,
				std::shared_ptr<MMPEngine::Backend::Vulkan::Wrapper::Fence>>(queue, allocator, cmdBuffer, fence)
	{
	}

	std::shared_ptr<Wrapper::CommandBuffer>& StreamContext::PopulateCommandsInBuffer()
	{
		if(!_commandsPopulated)
		{
			_commandsPopulated = true;

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			beginInfo.pInheritanceInfo = nullptr;
			beginInfo.pNext = nullptr;

			vkBeginCommandBuffer(_cmdBuffer->GetNative(), &beginInfo);
		}

		return _cmdBuffer;
	}

	void StreamContext::MarkCommandBufferAsPopulated()
	{
		_commandsPopulated = true;
	}

}