#pragma once
#include <Core/Stream.hpp>
#include <Backend/Shared/Context.hpp>
#include <Core/Passkey.hpp>

namespace MMPEngine::Backend::Shared
{
	template<typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	class Stream : public Core::Stream<TGlobalContext, StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>>
	{
	protected:
		using StreamContextType = StreamContext<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>;
		using PasskeyControl = typename StreamContextType::PasskeyControl;
		using Super = Core::Stream<TGlobalContext, StreamContextType>;

		Stream(const std::shared_ptr<TGlobalContext>& globalContext, const std::shared_ptr<StreamContextType>& streamContext);
		void RestartInternal() final;
		void SubmitInternal() final;
		void SyncInternal() final;

		virtual bool IsFenceCompleted() = 0;
		virtual void ResetCommandBufferAndAllocator() = 0;
		virtual void ScheduleCommandBufferForExecution() = 0;
		virtual void UpdateFence() = 0;
		virtual void WaitFence() = 0;
	protected:
		PasskeyControl _passKey;
	};

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	Stream<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::Stream(const std::shared_ptr<TGlobalContext>& globalContext, const std::shared_ptr<StreamContextType>& streamContext)
		: Super(globalContext, streamContext), _passKey(PasskeyControl {Core::PasskeyRequest {this}})
	{
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void Stream<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::RestartInternal()
	{
		Super::RestartInternal();

		if (IsFenceCompleted() && this->_specificStreamContext->IsCommandsClosed(_passKey))
		{
			ResetCommandBufferAndAllocator();
			this->_specificStreamContext->SetCommandsClosed(_passKey, false);
		}
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void Stream<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::SubmitInternal()
	{
		Super::SubmitInternal();

		if (this->_specificStreamContext->IsCommandsPopulated(_passKey))
		{
			if (!this->_specificStreamContext->IsCommandsClosed(_passKey))
			{
				ScheduleCommandBufferForExecution();
				this->_specificStreamContext->SetCommandsClosed(_passKey, true);
			}


			UpdateFence();
			this->_specificStreamContext->SetCommandsPopulated(_passKey, false);
		}
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void Stream<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::SyncInternal()
	{
		Super::SyncInternal();
		WaitFence();
	}
}