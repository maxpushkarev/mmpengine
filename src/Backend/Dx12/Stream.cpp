#include <Backend/Dx12/Stream.hpp>
#include <cassert>

namespace MMPEngine::Backend::Dx12
{
	Stream::Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext)
		: Super(globalContext, streamContext)
	{
	}
	bool Stream::ExecutionMonitorCompleted()
	{
		const auto fence = _specificStreamContext->GetFence(_passControl);
		const auto actualValue = fence->GetActualValue();
		const auto expectedValue = fence->GetExpectedValue();
		assert(actualValue <= expectedValue);
		return actualValue == expectedValue;
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
		fence->Signal(_specificStreamContext->GetQueue(), counterValue);
	}

	void Stream::WaitForExecutionMonitor()
	{
		const auto fence = _specificStreamContext->GetFence(_passControl);
		fence->Wait();
	}

	bool Stream::IsSyncCounterValueCompleted(std::uint64_t counterValue) const
	{
		return  _specificStreamContext->GetFence(_passControl)->GetActualValue() >= counterValue;
	}

}
