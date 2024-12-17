#include <Backend/Dx12/Stream.hpp>
#include <cassert>

namespace MMPEngine::Backend::Dx12
{
	Stream::Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext)
		: Super(globalContext, streamContext)
	{
		_waitHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		_fenceSignalValue = _specificStreamContext->_fence->GetCompletedValue();
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

			const auto fence = _specificStreamContext->_fence;
			const auto counterValue = GetSyncCounterValue();

			assert(_fenceSignalValue < counterValue);

			_fenceSignalValue = counterValue;
			_specificStreamContext->_cmdQueue->Signal(fence.Get(), _fenceSignalValue);
		}
	}


	void Stream::SyncInternal()
	{
		Super::SyncInternal();

		if (_specificStreamContext->_populatedCommands)
		{
			const auto fence = _specificStreamContext->_fence;

			if (fence->GetCompletedValue() < _fenceSignalValue)
			{
				fence->SetEventOnCompletion(_fenceSignalValue, _waitHandle);
				WaitForSingleObject(_waitHandle, INFINITE);
			}
		}
	}

	bool Stream::IsSyncCounterValueCompleted(std::uint64_t counterValue) const
	{
		return  _specificStreamContext->_fence->GetCompletedValue() >= counterValue;
	}

}
