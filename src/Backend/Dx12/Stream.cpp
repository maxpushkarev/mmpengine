#include <Backend/Dx12/Stream.hpp>
#include <cassert>

namespace MMPEngine::Backend::Dx12
{
	Stream::Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext)
		: Super(globalContext, streamContext)
	{
		_waitHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		_fenceSignalValue = _specificStreamContext->GetFence()->GetCompletedValue();
	}

	Stream::~Stream()
	{
		CloseHandle(_waitHandle);
	}

	bool Stream::IsFenceCompleted()
	{
		return _specificStreamContext->GetFence()->GetCompletedValue() == _fenceSignalValue;
	}

	void Stream::ResetCommandBufferAndAllocator()
	{
		_specificStreamContext->GetAllocator()->Reset();
		_specificStreamContext->GetCommandBuffer()->Reset(_specificStreamContext->GetAllocator().Get(), nullptr);
	}

	void Stream::ScheduleCommandBufferForExecution()
	{
		_specificStreamContext->GetCommandBuffer()->Close();
		ID3D12CommandList* cmdLists[]{ _specificStreamContext->GetCommandBuffer().Get() };
		_specificStreamContext->GetQueue()->ExecuteCommandLists(static_cast<std::uint32_t>(std::size(cmdLists)), cmdLists);
	}

	void Stream::UpdateFence()
	{
		const auto fence = _specificStreamContext->GetFence();
		const auto counterValue = GetSyncCounterValue();

		assert(_fenceSignalValue < counterValue);
		_fenceSignalValue = counterValue;
		_specificStreamContext->GetQueue()->Signal(fence.Get(), _fenceSignalValue);
	}

	void Stream::WaitFence()
	{
		if (_specificStreamContext->GetFence()->GetCompletedValue() < _fenceSignalValue)
		{
			_specificStreamContext->GetFence()->SetEventOnCompletion(_fenceSignalValue, _waitHandle);
			WaitForSingleObject(_waitHandle, INFINITE);
		}
	}

	bool Stream::IsSyncCounterValueCompleted(std::uint64_t counterValue) const
	{
		return  _specificStreamContext->GetFence()->GetCompletedValue() >= counterValue;
	}

}
