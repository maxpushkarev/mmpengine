#include <Backend/Dx12/Stream.hpp>
#include <cassert>

namespace MMPEngine::Backend::Dx12
{
	Stream::Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext)
		: Super(globalContext, streamContext)
	{
		_waitHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		_fenceSignalValue = _specificStreamContext->GetFence(_passControl)->GetCompletedValue();
	}

	Stream::~Stream()
	{
		CloseHandle(_waitHandle);
	}

	bool Stream::ExecutionMonitorCompleted()
	{
		return _specificStreamContext->GetFence(_passControl)->GetCompletedValue() == _fenceSignalValue;
	}

	void Stream::ResetAll()
	{
		_specificStreamContext->GetAllocator(_passControl)->Reset();
		_specificStreamContext->GetCommandBuffer(_passControl)->Reset(_specificStreamContext->GetAllocator(_passControl).Get(), nullptr);
	}

	void Stream::ScheduleCommandsForExecution()
	{
		_specificStreamContext->GetCommandBuffer(_passControl)->Close();
		ID3D12CommandList* cmdLists[]{ _specificStreamContext->GetCommandBuffer(_passControl).Get() };
		_specificStreamContext->GetQueue()->ExecuteCommandLists(static_cast<std::uint32_t>(std::size(cmdLists)), cmdLists);
	}

	void Stream::UpdateExecutionMonitor()
	{
		const auto fence = _specificStreamContext->GetFence(_passControl);
		const auto counterValue = GetSyncCounterValue();

		assert(_fenceSignalValue < counterValue);
		_fenceSignalValue = counterValue;
		_specificStreamContext->GetQueue()->Signal(fence.Get(), _fenceSignalValue);
	}

	void Stream::WaitForExecutionMonitor()
	{
		if (_specificStreamContext->GetFence(_passControl)->GetCompletedValue() < _fenceSignalValue)
		{
			_specificStreamContext->GetFence(_passControl)->SetEventOnCompletion(_fenceSignalValue, _waitHandle);
			WaitForSingleObject(_waitHandle, INFINITE);
		}
	}

	bool Stream::IsSyncCounterValueCompleted(std::uint64_t counterValue) const
	{
		return  _specificStreamContext->GetFence(_passControl)->GetCompletedValue() >= counterValue;
	}

}
