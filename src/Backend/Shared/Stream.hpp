#pragma once
#include <Core/Stream.hpp>
#include <Backend/Shared/Context.hpp>
#include <Core/Passkey.hpp>

namespace MMPEngine::Backend::Shared
{
	template<typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	class Stream : public Core::Stream<TGlobalContext, StreamContext<TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>>
	{
	protected:
		using StreamContextType = StreamContext<TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>;
		using PassControl = typename StreamContextType::PassControl;
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
		PassControl _passControl;
	};

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	Stream<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::Stream(const std::shared_ptr<TGlobalContext>& globalContext, const std::shared_ptr<StreamContextType>& streamContext)
		: Super(globalContext, streamContext), _passControl(PassControl {Core::PassKey {this}})
	{
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void Stream<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::RestartInternal()
	{
		Super::RestartInternal();

		if (IsFenceCompleted() && this->_specificStreamContext->IsCommandsClosed(_passControl))
		{
			ResetCommandBufferAndAllocator();
			this->_specificStreamContext->SetCommandsClosed(_passControl, false);
		}
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void Stream<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::SubmitInternal()
	{
		Super::SubmitInternal();

		if (this->_specificStreamContext->IsCommandsPopulated(_passControl))
		{
			if (!this->_specificStreamContext->IsCommandsClosed(_passControl))
			{
				ScheduleCommandBufferForExecution();
				this->_specificStreamContext->SetCommandsClosed(_passControl, true);
			}


			UpdateFence();
			this->_specificStreamContext->SetCommandsPopulated(_passControl, false);
		}
	}

	template <typename TGlobalContext, typename TQueue, typename TCommandBufferAllocator, typename TCommandBuffer, typename TFence>
	void Stream<TGlobalContext, TQueue, TCommandBufferAllocator, TCommandBuffer, TFence>::SyncInternal()
	{
		Super::SyncInternal();
		WaitFence();
	}
}