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

	bool Stream::IsFenceCompleted()
	{
		return _specificStreamContext->_fence->GetCompletedValue() == _fenceSignalValue;
	}

	void Stream::ResetCommandBufferAndAllocator()
	{
		_specificStreamContext->_allocator->Reset();
		_specificStreamContext->_cmdBuffer->Reset(_specificStreamContext->_allocator.Get(), nullptr);
	}

	void Stream::ScheduleCommandBufferForExecution()
	{
		_specificStreamContext->_cmdBuffer->Close();
		ID3D12CommandList* cmdLists[]{ _specificStreamContext->_cmdBuffer.Get() };
		_specificStreamContext->_queue->ExecuteCommandLists(static_cast<std::uint32_t>(std::size(cmdLists)), cmdLists);
	}

	void Stream::UpdateFence()
	{
		const auto fence = _specificStreamContext->_fence;
		const auto counterValue = GetSyncCounterValue();

		assert(_fenceSignalValue < counterValue);
		_fenceSignalValue = counterValue;
		_specificStreamContext->_queue->Signal(fence.Get(), _fenceSignalValue);
	}

	void Stream::WaitFence()
	{
		if (_specificStreamContext->_fence->GetCompletedValue() < _fenceSignalValue)
		{
			_specificStreamContext->_fence->SetEventOnCompletion(_fenceSignalValue, _waitHandle);
			WaitForSingleObject(_waitHandle, INFINITE);
		}
	}

	bool Stream::IsSyncCounterValueCompleted(std::uint64_t counterValue) const
	{
		return  _specificStreamContext->_fence->GetCompletedValue() >= counterValue;
	}

}
