#pragma once
#include <Core/Context.hpp>

namespace MMPEngine::Backend::Shared
{
	template<typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	class Stream;

	template<typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	class StreamContext : public Core::StreamContext
	{
		friend Stream<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>;
	public:
		StreamContext(
			const std::shared_ptr<TGlobalContext>& globalContext,
			TQueue queue,
			TCommandBufferAllocator allocator,
			TCommandBuffer cmdBuffer,
			TFence fence
		);
		TCommandBuffer& PopulateCommandsInList();
		TQueue& GetQueue();
	private:
		std::shared_ptr<TGlobalContext> _globalContext;
		TQueue _queue;
		TCommandBufferAllocator _allocator;
		TCommandBuffer _cmdBuffer;
		TFence _fence;
		bool _commandsPopulated = false;
		bool _commandsClosed = true;
	};

	template<typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::StreamContext(
		const std::shared_ptr<TGlobalContext>& globalContext,
		TQueue queue,
		TCommandBufferAllocator allocator,
		TCommandBuffer cmdBuffer,
		TFence fence
	) : _globalContext(globalContext), _queue(queue), _allocator(allocator), _cmdBuffer(cmdBuffer), _fence(fence)
	{
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	TQueue& StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::GetQueue()
	{
		return _queue;
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	TCommandBuffer& StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::PopulateCommandsInList()
	{
		return  _cmdBuffer;
	}
}