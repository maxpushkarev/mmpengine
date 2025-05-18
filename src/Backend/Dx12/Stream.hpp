#pragma once
#include <Core/Stream.hpp>
#include <Backend/Dx12/Context.hpp>
#include <Backend/Shared/Stream.hpp>
#include <Backend/Dx12/Wrapper.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Stream : public Shared::Stream<
		GlobalContext,
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>,
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>,
		std::shared_ptr<Wrapper::Fence>>
	{
	private:
		using Super = Shared::Stream<
			GlobalContext,
			Microsoft::WRL::ComPtr<ID3D12CommandQueue>,
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator>,
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>,
			std::shared_ptr<Wrapper::Fence>>;
	public:
		Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext);
		bool IsSyncCounterValueCompleted(std::uint64_t counterValue) const override;
	protected:
		bool ExecutionMonitorCompleted() override;
		void ResetAll() override;
		void ScheduleCommandsForExecution() override;
		void UpdateExecutionMonitor() override;
		void WaitForExecutionMonitor() override;
	private:
		std::vector<ID3D12DescriptorHeap*> _heaps;
	};
}