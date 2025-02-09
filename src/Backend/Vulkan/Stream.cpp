#include <Backend/Vulkan/Stream.hpp>

namespace MMPEngine::Backend::Vulkan
{
	Stream::Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext)
		: Super(globalContext, streamContext)
	{
	}

	Stream::~Stream() = default;

	bool Stream::IsFenceCompleted()
	{
		return vkGetFenceStatus(_specificGlobalContext->device->GetNative(), _specificStreamContext->GetFence(_passControl)->GetNative()) == VK_SUCCESS;
	}

	void Stream::ResetCommandBufferAndAllocator()
	{
		vkResetCommandBuffer(_specificStreamContext->GetCommandBuffer(_passControl)->GetNative(), VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	}

	void Stream::ScheduleCommandBufferForExecution()
	{
		
	}

	void Stream::UpdateFence()
	{
		
	}

	void Stream::WaitFence()
	{
		
	}
}