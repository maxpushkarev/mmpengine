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

	void Stream::WaitInternal()
	{
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
