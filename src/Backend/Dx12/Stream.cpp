#include <Backend/Dx12/Stream.hpp>

namespace MMPEngine::Backend::Dx12
{
	Stream::Stream(const std::shared_ptr<AppContext>& appContext, const std::shared_ptr<StreamContext>& streamContext)
		: Super(appContext, streamContext)
	{
		_waitHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		_lastFenceValue = _specificStreamContext->fence->GetCompletedValue();
	}

	Stream::~Stream()
	{
		CloseHandle(_waitHandle);
	}

	void Stream::RestartInternal()
	{
		Core::Stream<AppContext, StreamContext>::RestartInternal();

		_specificStreamContext->cmdAllocator->Reset();
		_specificStreamContext->cmdList->Reset(_specificStreamContext->cmdAllocator.Get(), nullptr);
	}


	void Stream::SubmitInternal()
	{
		Core::Stream<AppContext, StreamContext>::SubmitInternal();

		_specificStreamContext->cmdList->Close();
		ID3D12CommandList* cmdLists[] { _specificStreamContext->cmdList.Get() };
		_specificStreamContext->cmdQueue->ExecuteCommandLists(std::size(cmdLists), cmdLists);
	}


	void Stream::WaitInternal()
	{
		Core::Stream<AppContext, StreamContext>::WaitInternal();

		const auto fence = _specificStreamContext->fence;

		++_lastFenceValue;
		_specificStreamContext->cmdQueue->Signal(fence.Get(), _lastFenceValue);

		if (fence->GetCompletedValue() < _lastFenceValue)
		{
			fence->SetEventOnCompletion(_lastFenceValue, _waitHandle);
			WaitForSingleObject(_waitHandle, INFINITE);
		}

		Super::WaitInternal();
	}

}
