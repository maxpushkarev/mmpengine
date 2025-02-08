#pragma once
#include <Core/Stream.hpp>
#include <Backend/Shared/Context.hpp>

namespace MMPEngine::Backend::Shared
{
	template<typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	class Stream : public Core::Stream<TGlobalContext, StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>>
	{
	private:
		using StreamContextType = StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>;
		using Super = Core::Stream<TGlobalContext, StreamContextType>;
	protected:
		Stream(const std::shared_ptr<TGlobalContext>& globalContext, const std::shared_ptr<StreamContextType>& streamContext);
		void RestartInternal() final;
		void SubmitInternal() final;
		void SyncInternal() final;

		virtual bool IsFenceCompleted() = 0;
		virtual void ResetCommandBufferAndAllocator() = 0;
		virtual void ScheduleCommandBufferForExecution() = 0;
		virtual void UpdateFence() = 0;
		virtual void WaitFence() = 0;
	};

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	Stream<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::Stream(const std::shared_ptr<TGlobalContext>& globalContext, const std::shared_ptr<StreamContextType>& streamContext)
		: Super(globalContext, streamContext)
	{
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void Stream<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::RestartInternal()
	{
		Super::RestartInternal();

		if (IsFenceCompleted() && this->_specificStreamContext->_commandsClosed)
		{
			ResetCommandBufferAndAllocator();
		}
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void Stream<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::SubmitInternal()
	{
		Super::SubmitInternal();

		if (this->_specificStreamContext->_commandsPopulated)
		{
			if (!this->_specificStreamContext->_commandsClosed)
			{
				ScheduleCommandBufferForExecution();
				this->_specificStreamContext->_commandsClosed = true;
			}


			UpdateFence();

			this->_specificStreamContext->_commandsPopulated = false;
		}
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void Stream<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::SyncInternal()
	{
		Super::SyncInternal();

		WaitFence();
	}
}