#pragma once
#include <Core/Camera.hpp>
#include <Backend/Dx12/Task.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Camera
	{
	public:

		class DrawCallsJob;

		Camera(const Camera&) = delete;
		Camera(Camera&&) noexcept = delete;
		Camera& operator=(const Camera&) = delete;
		Camera& operator=(Camera&&) noexcept = delete;
		virtual ~Camera();
	protected:
		Camera();

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

		static void FillNonProjectionData(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::shared_ptr<Core::Node>& node, Core::Camera::Data& data);
	};

	class PerspectiveCamera final : public Core::PerspectiveCamera, public Camera
	{
	public:
		PerspectiveCamera(const Settings& settings, const std::shared_ptr<Core::Node>& node, const Target& target);
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
		OrthographicCamera(const Settings& settings, const std::shared_ptr<Core::Node>& node, const Target& target);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	protected:
		void FillData(const std::shared_ptr<Core::GlobalContext>& globalContext, Data& data) override;
		std::shared_ptr<Core::UniformBuffer<Core::Camera::Data>>& GetUniformBufferRef() override;
		std::shared_ptr<Core::ContextualTask<Core::UniformBuffer<Core::Camera::Data>::WriteTaskContext>>& GetUniformBufferUpdateTaskRef() override;
		void FillDataInternal(const std::shared_ptr<Core::GlobalContext>& globalContext, Core::Camera::Data& data) override;
	};
}
