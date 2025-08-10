#pragma once
#include <Core/Stream.hpp>
#include <Backend/Dx12/Context.hpp>
#include <Backend/Shared/Stream.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Stream : public Shared::Stream<
		GlobalContext,
		StreamContext>
	{
	private:
		using Super = Shared::Stream<
			GlobalContext,
			StreamContext>;
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