#pragma once
#include <Core/DrawCall.hpp>
#include <Backend/Dx12/Camera.hpp>
#include <Backend/Dx12/Task.hpp>
#include <Backend/Dx12/Texture.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Camera::DrawCallsJob final : public Core::Camera::DrawCallsJob
	{
	private:
		class InternalTaskContext final : public Core::TaskContext
		{
		public:
			std::shared_ptr<DrawCallsJob> job;
			std::vector<std::shared_ptr<BaseEntity>> colorRenderTargets;
			std::shared_ptr<BaseEntity> depthStencil;
		};

		class PrepareRenderTargetsTask final : public Task<InternalTaskContext>
		{
		public:
			PrepareRenderTargetsTask(const std::shared_ptr<InternalTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class PrepareTask final : public Task<InternalTaskContext>
		{
		public:
			PrepareTask(const std::shared_ptr<InternalTaskContext>& ctx);
		protected:
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		private:
			std::shared_ptr<PrepareRenderTargetsTask> _prepareRenderTargets;
			std::shared_ptr<BindDescriptorPoolsTask> _bindDescriptorHeaps;
			std::vector<std::shared_ptr<Core::BaseTask>> _switchStateTasks;
		};

	public:
		DrawCallsJob(const std::shared_ptr<Core::Camera>& camera, std::vector<Item>&& items);
	protected:
		std::shared_ptr<Iteration> BuildIteration(const Item& item) const override;
		std::shared_ptr<Core::BaseTask> CreateTaskForIterationsStart() override;
	};
}