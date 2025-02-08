#pragma once
#include <Core/Context.hpp>

namespace MMPEngine::Backend::Shared
{
	template<typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	class StreamContext : public Core::StreamContext
	{
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
		TCommandBufferAllocator& GetAllocator();
		TCommandBuffer& GetCommandBuffer();
		TFence& GetFence();

		bool IsCommandsPopulated() const;
		bool IsCommandsClosed() const;

		void SetCommandsPopulated(bool value);
		void SetCommandsClosed(bool value);

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
		_commandsPopulated = true;
		return _cmdBuffer;
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	TCommandBuffer& StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::GetCommandBuffer()
	{
		return _cmdBuffer;
	}


	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	TCommandBufferAllocator& StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::GetAllocator()
	{
		return _allocator;
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	TFence& StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::GetFence()
	{
		return _fence;
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	bool StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::IsCommandsClosed() const
	{
		return _commandsClosed;
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	bool StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::IsCommandsPopulated() const
	{
		return _commandsPopulated;
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::SetCommandsClosed(bool value)
	{
		_commandsClosed = value;
	}


	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::SetCommandsPopulated(bool value)
	{
		_commandsPopulated = value;
	}

}