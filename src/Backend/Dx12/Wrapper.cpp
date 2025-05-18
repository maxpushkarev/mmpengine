#include <Backend/Dx12/Wrapper.hpp>
#include <cassert>

namespace MMPEngine::Backend::Dx12
{
	namespace Wrapper
	{
		Fence::Fence(const Microsoft::WRL::ComPtr<ID3D12Device>& device)
		{
			device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
				IID_PPV_ARGS(_fence.GetAddressOf()));

			_waitHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
			_expectedValue = 0;
		}

		Fence::~Fence()
		{
			CloseHandle(_waitHandle);
		}

		void Fence::Signal(const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& queue)
		{
			Signal(queue, GetExpectedValue() + 1);
		}

		void Fence::Signal(const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& queue, std::uint64_t newVal)
		{
			assert(newVal > _fence->GetCompletedValue());
			assert(newVal > _expectedValue);

			_expectedValue = newVal;
			queue->Signal(_fence.Get(), _expectedValue);
		}

		void Fence::Wait()
		{
			if (_fence->GetCompletedValue() < _expectedValue)
			{
				_fence->SetEventOnCompletion(_expectedValue, _waitHandle);
				WaitForSingleObject(_waitHandle, INFINITE);
			}

		}


		std::uint64_t Fence::GetExpectedValue() const
		{
			return _expectedValue;
		}

		std::uint64_t Fence::GetActualValue() const
		{
			return _fence->GetCompletedValue();
		}

	}
}