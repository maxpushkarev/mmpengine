#pragma once
#include <Core/DrawCall.hpp>
#include <Frontend/Camera.hpp>

namespace MMPEngine::Frontend
{
	class Camera::DrawCallsJob final : public Core::Camera::DrawCallsJob
	{
	public:
		DrawCallsJob(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::shared_ptr<Core::Camera>& camera, std::vector<Item>&& items);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateExecutionTask() override;
	protected:
		std::shared_ptr<Iteration> BuildIteration(const Item& item) const override;
		std::shared_ptr<Core::BaseTask> CreateTaskForIterationsStart() override;
		std::shared_ptr<Core::BaseTask> CreateTaskForIterationsFinish() override;
	private:
		std::shared_ptr<Core::Camera::DrawCallsJob> _impl;
	};
}