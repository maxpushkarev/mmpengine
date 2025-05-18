#pragma once
#include <memory>
#include <d3d12.h>
#include <wrl/client.h>

namespace MMPEngine::Backend::Dx12
{
	namespace Wrapper
	{
		class Fence final
		{
		public:
			Fence(const Microsoft::WRL::ComPtr<ID3D12Device>& device);
			Fence(const Fence&) = delete;
			Fence(Fence&&) noexcept = delete;
			Fence& operator=(const Fence&) = delete;
			Fence& operator=(Fence&&) noexcept = delete;
			~Fence();

			std::uint64_t GetExpectedValue() const;
			std::uint64_t GetActualValue() const;
			void Signal(const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& queue);
			void Signal(const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& queue, std::uint64_t newVal);
			void Wait();

		private:
			std::uint64_t _expectedValue;
			Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
			HANDLE _waitHandle;
		};
	}
}