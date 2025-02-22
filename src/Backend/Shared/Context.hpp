#pragma once
#include <Core/Context.hpp>
#include <Core/Passkey.hpp>
#include <Core/Stream.hpp>

namespace MMPEngine::Backend::Shared
{
	template<typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	class StreamContext : public Core::StreamContext
	{
	public:
		using PassControl = Core::PassControl<true, Core::BaseStream>;
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
		TFence& GetFence(PassControl);

		bool IsCommandsPopulated(PassControl) const;
		bool IsCommandsClosed(PassControl) const;

		void SetCommandsPopulated(PassControl, bool value);
		void SetCommandsClosed(PassControl, bool value);
	protected:
		bool _commandsPopulated = false;
		bool _commandsClosed = true;
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
	TFence& StreamContext<TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::GetFence(PassControl)
	{
		return _fence;
	}

	template <typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	bool StreamContext<TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::IsCommandsClosed(PassControl) const
	{
		return _commandsClosed;
	}

	template <typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	bool StreamContext<TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::IsCommandsPopulated(PassControl) const
	{
		return _commandsPopulated;
	}

	template <typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void StreamContext<TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::SetCommandsClosed(PassControl, bool value)
	{
		_commandsClosed = value;
	}


	template <typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void StreamContext<TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::SetCommandsPopulated(PassControl, bool value)
	{
		_commandsPopulated = value;
	}

}