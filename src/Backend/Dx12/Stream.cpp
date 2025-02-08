#include <Backend/Dx12/Stream.hpp>
#include <cassert>

namespace MMPEngine::Backend::Dx12
{
	Stream::Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext)
		: Super(globalContext, streamContext)
	{
		_waitHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		_fenceSignalValue = _specificStreamContext->GetFence(_passKey)->GetCompletedValue();
	}

	Stream::~Stream()
	{
		CloseHandle(_waitHandle);
	}

	bool Stream::IsFenceCompleted()
	{
		return _specificStreamContext->GetFence(_passKey)->GetCompletedValue() == _fenceSignalValue;
	}

	void Stream::ResetCommandBufferAndAllocator()
	{
		_specificStreamContext->GetAllocator(_passKey)->Reset();
		_specificStreamContext->GetCommandBuffer(_passKey)->Reset(_specificStreamContext->GetAllocator(_passKey).Get(), nullptr);
	}

	void Stream::ScheduleCommandBufferForExecution()
	{
		_specificStreamContext->GetCommandBuffer(_passKey)->Close();
		ID3D12CommandList* cmdLists[]{ _specificStreamContext->GetCommandBuffer(_passKey).Get() };
		_specificStreamContext->GetQueue()->ExecuteCommandLists(static_cast<std::uint32_t>(std::size(cmdLists)), cmdLists);
	}

	void Stream::UpdateFence()
	{
		const auto fence = _specificStreamContext->GetFence(_passKey);
		const auto counterValue = GetSyncCounterValue();

		assert(_fenceSignalValue < counterValue);
		_fenceSignalValue = counterValue;
		_specificStreamContext->GetQueue()->Signal(fence.Get(), _fenceSignalValue);
	}

	void Stream::WaitFence()
	{
		if (_specificStreamContext->GetFence(_passKey)->GetCompletedValue() < _fenceSignalValue)
		{
			_specificStreamContext->GetFence(_passKey)->SetEventOnCompletion(_fenceSignalValue, _waitHandle);
			WaitForSingleObject(_waitHandle, INFINITE);
		}
	}

	bool Stream::IsSyncCounterValueCompleted(std::uint64_t counterValue) const
	{
		return  _specificStreamContext->GetFence(_passKey)->GetCompletedValue() >= counterValue;
	}

}
