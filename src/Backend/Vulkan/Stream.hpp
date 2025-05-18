#pragma once
#include <Core/Stream.hpp>
#include <Backend/Vulkan/Context.hpp>
#include <Backend/Vulkan/Wrapper.hpp>
#include <Backend/Shared/Stream.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class Stream : public Shared::Stream<
		GlobalContext,
		std::shared_ptr<Wrapper::Queue>,
		std::shared_ptr<Wrapper::CommandAllocator>,
		std::shared_ptr<Wrapper::CommandBuffer>,
		std::shared_ptr<Wrapper::Fence>>
	{
	private:
		using Super = Shared::Stream<
			GlobalContext,
			std::shared_ptr<Wrapper::Queue>,
			std::shared_ptr<Wrapper::CommandAllocator>,
			std::shared_ptr<Wrapper::CommandBuffer>,
			std::shared_ptr<Wrapper::Fence>>;
	public:
		Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext);
		Stream(const Stream&) = delete;
		Stream(Stream&&) noexcept = delete;
		Stream& operator=(const Stream&) = delete;
		Stream& operator=(Stream&&) noexcept = delete;
		~Stream() override;
	protected:
		bool ExecutionMonitorCompleted() override;
		void ResetAll() override;
		void ScheduleCommandsForExecution() override;
		void UpdateExecutionMonitor() override;
		void WaitForExecutionMonitor() override;
	};
}