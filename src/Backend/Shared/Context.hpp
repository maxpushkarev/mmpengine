#pragma once
#include <Core/Context.hpp>
#include <Core/Passkey.hpp>
#include <Core/Stream.hpp>

namespace MMPEngine::Backend::Shared
{
	template<typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	class StreamContext : public Core::StreamContext
	{
	public:
		using PasskeyControl = Core::PasskeyControl<true, Core::BaseStream>;
		StreamContext(
			const std::shared_ptr<TGlobalContext>& globalContext,
			TQueue queue,
			TCommandBufferAllocator allocator,
			TCommandBuffer cmdBuffer,
			TFence fence
		);
		TCommandBuffer& PopulateCommandsInList();
		TQueue& GetQueue();
		TCommandBufferAllocator& GetAllocator(PasskeyControl);
		TCommandBuffer& GetCommandBuffer(PasskeyControl);
		TFence& GetFence(PasskeyControl);

		bool IsCommandsPopulated(PasskeyControl) const;
		bool IsCommandsClosed(PasskeyControl) const;

		void SetCommandsPopulated(PasskeyControl, bool value);
		void SetCommandsClosed(PasskeyControl, bool value);

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
	TCommandBuffer& StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::GetCommandBuffer(PasskeyControl)
	{
		return _cmdBuffer;
	}


	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	TCommandBufferAllocator& StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::GetAllocator(PasskeyControl)
	{
		return _allocator;
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	TFence& StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::GetFence(PasskeyControl)
	{
		return _fence;
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	bool StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::IsCommandsClosed(PasskeyControl) const
	{
		return _commandsClosed;
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	bool StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::IsCommandsPopulated(PasskeyControl) const
	{
		return _commandsPopulated;
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::SetCommandsClosed(PasskeyControl, bool value)
	{
		_commandsClosed = value;
	}


	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::SetCommandsPopulated(PasskeyControl, bool value)
	{
		_commandsPopulated = value;
	}

}