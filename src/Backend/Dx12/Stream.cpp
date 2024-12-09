#include <Backend/Dx12/Stream.hpp>

namespace MMPEngine::Backend::Dx12
{
	Stream::Stream(const std::shared_ptr<AppContext>& appContext, const std::shared_ptr<StreamContext>& streamContext)
		: Super(appContext, streamContext)
	{
		_waitHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		_lastFenceValue = _specificStreamContext->_fence->GetCompletedValue();
	}

	Stream::~Stream()
	{
		CloseHandle(_waitHandle);
	}

	void Stream::RestartInternal()
	{
		Super::RestartInternal();

		_specificStreamContext->_cmdAllocator->Reset();
		_specificStreamContext->_cmdList->Reset(_specificStreamContext->_cmdAllocator.Get(), nullptr);
		_specificStreamContext->_populatedCommands = false;
	}


	void Stream::SubmitInternal()
	{
		Super::SubmitInternal();

		_specificStreamContext->_cmdList->Close();

		if(_specificStreamContext->_populatedCommands)
		{
			ID3D12CommandList* cmdLists[]{ _specificStreamContext->_cmdList.Get() };
			_specificStreamContext->_cmdQueue->ExecuteCommandLists(static_cast<std::uint32_t>(std::size(cmdLists)), cmdLists);
		}
	}


	void Stream::SyncInternal()
	{
		Super::SyncInternal();

		if (_specificStreamContext->_populatedCommands)
		{
			const auto fence = _specificStreamContext->_fence;

			++_lastFenceValue;
			_specificStreamContext->_cmdQueue->Signal(fence.Get(), _lastFenceValue);

			if (fence->GetCompletedValue() < _lastFenceValue)
			{
				fence->SetEventOnCompletion(_lastFenceValue, _waitHandle);
				WaitForSingleObject(_waitHandle, INFINITE);
			}
		}
	}

}
