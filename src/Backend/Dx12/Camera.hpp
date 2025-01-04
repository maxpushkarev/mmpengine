#pragma once
#include <Core/Camera.hpp>
#include <Backend/Dx12/Task.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Camera
	{
	protected:
		class CameraTaskContext final : public Core::EntityTaskContext<Camera>
		{
		};

		class InitTask final : public Task<CameraTaskContext>
		{
		public:
			InitTask(const std::shared_ptr<CameraTaskContext>& ctx);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		virtual void FillDataInternal(const std::shared_ptr<Core::GlobalContext>& globalContext, Core::Camera::Data& data) = 0;
		virtual std::shared_ptr<Core::UniformBuffer<Core::Camera::Data>>& GetUniformBufferRef() = 0;
		virtual std::shared_ptr<Core::ContextualTask<Core::UniformBuffer<Core::Camera::Data>::WriteTaskContext>>& GetUniformBufferUpdateTaskRef() = 0;
	};

	class PerspectiveCamera final : public Core::PerspectiveCamera, public Camera
	{
	public:
		PerspectiveCamera(const Settings& settings, const std::shared_ptr<Core::Node>& node);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	protected:
		void FillData(const std::shared_ptr<Core::GlobalContext>& globalContext, Data& data) override;
		std::shared_ptr<Core::UniformBuffer<Core::Camera::Data>>& GetUniformBufferRef() override;
		std::shared_ptr<Core::ContextualTask<Core::UniformBuffer<Core::Camera::Data>::WriteTaskContext>>& GetUniformBufferUpdateTaskRef() override;
		void FillDataInternal(const std::shared_ptr<Core::GlobalContext>& globalContext, Core::Camera::Data& data) override;
	};

	class OrthographicCamera final : public Core::OrthographicCamera, public Camera
	{
	public:
		OrthographicCamera(const Settings& settings, const std::shared_ptr<Core::Node>& node);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	protected:
		void FillData(const std::shared_ptr<Core::GlobalContext>& globalContext, Data& data) override;
		std::shared_ptr<Core::UniformBuffer<Core::Camera::Data>>& GetUniformBufferRef() override;
		std::shared_ptr<Core::ContextualTask<Core::UniformBuffer<Core::Camera::Data>::WriteTaskContext>>& GetUniformBufferUpdateTaskRef() override;
		void FillDataInternal(const std::shared_ptr<Core::GlobalContext>& globalContext, Core::Camera::Data& data) override;
	};
}
