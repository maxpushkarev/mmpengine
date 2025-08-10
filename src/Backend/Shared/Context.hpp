#pragma once
#include <Core/Context.hpp>
#include <Core/Passkey.hpp>
#include <Core/Stream.hpp>

namespace MMPEngine::Backend::Shared
{
    class BaseStreamContext : public Core::StreamContext
    {
    public:
        using PassControl = Core::PassControl<true, Core::BaseStream>;
        
        bool IsCommandsPopulated(PassControl) const;
        bool IsCommandsClosed(PassControl) const;

        void SetCommandsPopulated(PassControl, bool value);
        void SetCommandsClosed(PassControl, bool value);
        
    protected:
        bool _commandsPopulated = false;
        bool _commandsClosed = true;
    };

	template<typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	class StreamContext : public BaseStreamContext
	{
	public:
		StreamContext(
			const TQueue& queue,
			const TCommandBufferAllocator& allocator,
			const TCommandBuffer& cmdBuffer,
			const TFence& fence
		);
		virtual TCommandBuffer& PopulateCommandsInBuffer();
		TQueue& GetQueue();
		TCommandBufferAllocator& GetAllocator(PassControl);
		TCommandBuffer& GetCommandBuffer(PassControl);
		TFence& GetFence();
        
	protected:
		TQueue _queue;
		TCommandBufferAllocator _allocator;
		TCommandBuffer _cmdBuffer;
		TFence _fence;
	};

	template<typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	StreamContext<TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::StreamContext(
		const TQueue& queue,
		const TCommandBufferAllocator& allocator,
		const TCommandBuffer& cmdBuffer,
		const TFence& fence
	) : _queue(queue), _allocator(allocator), _cmdBuffer(cmdBuffer), _fence(fence)
	{
	}

	template <typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	TQueue& StreamContext<TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::GetQueue()
	{
		return _queue;
	}

	template <typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	TCommandBuffer& StreamContext<TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::PopulateCommandsInBuffer()
	{
		_commandsPopulated = true;
		return _cmdBuffer;
	}

	template <typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	TCommandBuffer& StreamContext<TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::GetCommandBuffer(PassControl)
	{
		return _cmdBuffer;
	}


	template <typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	TCommandBufferAllocator& StreamContext<TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::GetAllocator(PassControl)
	{
		return _allocator;
	}

	template <typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	TFence& StreamContext<TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::GetFence()
	{
		return _fence;
	}
}
