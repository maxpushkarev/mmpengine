#pragma once
#include <Core/DrawCall.hpp>
#include <Backend/Dx12/Camera.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Camera::DrawCallsJob final : public Core::Camera::DrawCallsJob
	{
	public:
		DrawCallsJob(const std::shared_ptr<Core::Camera>& camera, std::vector<Item>&& items);
	protected:
		std::shared_ptr<Iteration> BuildIteration(const Item& item) const override;
		std::shared_ptr<Core::BaseTask> CreateTaskForIterationsStart() override;
	};
}