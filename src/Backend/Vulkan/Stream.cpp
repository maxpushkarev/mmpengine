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
		return vkGetFenceStatus(_specificGlobalContext->device->GetNativeLogical(), _specificStreamContext->GetFence(_passControl)->GetNative()) == VK_SUCCESS;
	}

	void Stream::ResetCommandBufferAndAllocator()
	{
		vkResetCommandBuffer(_specificStreamContext->GetCommandBuffer(_passControl)->GetNative(), VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	}

	void Stream::ScheduleCommandBufferForExecution()
	{
		const auto cb = _specificStreamContext->GetCommandBuffer(_passControl)->GetNative()

		VkSubmitInfo submitInfo;
		submitInfo.pNext = nullptr;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cb;

		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;

		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;

		vkEndCommandBuffer(cb);
		vkQueueSubmit(
			_specificStreamContext->GetQueue()->GetNative(),
			1,
			&submitInfo,
			nullptr
		);
	}

	void Stream::UpdateFence()
	{
		VkSubmitInfo submitInfo;
		submitInfo.pNext = nullptr;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		submitInfo.commandBufferCount = 0;
		submitInfo.pCommandBuffers = nullptr;

		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;

		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;

		const auto vkFence = _specificStreamContext->GetFence(_passControl)->GetNative();

		vkResetFences(_specificGlobalContext->device->GetNativeLogical(), 1, &vkFence);
		vkQueueSubmit(
			_specificStreamContext->GetQueue()->GetNative(),
			1,
			&submitInfo,
			vkFence
		);
	}

	void Stream::WaitFence()
	{
		const auto vkFence = _specificStreamContext->GetFence(_passControl)->GetNative();
		vkWaitForFences(
			_specificGlobalContext->device->GetNativeLogical(),
			1,
			&vkFence,
			VK_TRUE,
			(std::numeric_limits<std::uint64_t>::max)()
		);
	}
}