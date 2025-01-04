#pragma once
#include <Core/Entity.hpp>
#include <Core/Material.hpp>
#include <Core/Buffer.hpp>
#include <Core/Node.hpp>

namespace MMPEngine::Core
{
	class Camera : public BaseEntity
	{
	public:

		struct Settings final
		{
			std::float_t nearPlane = 0.1f;
			std::float_t farPlane = 100.0f;
			bool clearDepthStencil = true;
			std::array<bool, RenderingMaterial::Settings::Blend::kMaxRenderTargets> clearColor {};
			std::string name {};
		};

		struct Data final
		{
			Matrix4x4 viewMatrix;
			Matrix4x4 projMatrix;
			Vector4Float worldPosition;
		};
	private:

		class UpdateDataTaskContext : public TaskContext
		{
		public:
			std::optional<Data> precomputed = std::nullopt;
		};

		class InternalUpdateDataTaskContext final : public UpdateDataTaskContext
		{
		public:
			std::shared_ptr<Camera> camera;
		};

		class InternalUpdateDataTask final : public ContextualTask<UpdateDataTaskContext>
		{
		public:
			InternalUpdateDataTask(const std::shared_ptr<InternalUpdateDataTaskContext>& ctx);
			void OnScheduled(const std::shared_ptr<BaseStream>& stream) override;
		private:
			std::shared_ptr<InternalUpdateDataTaskContext> _internalContext;
		};

	public:
		virtual std::shared_ptr<BaseEntity> GetUniformDataEntity() const;
		virtual std::shared_ptr<Node> GetNode() const;
		virtual std::shared_ptr<ContextualTask<UpdateDataTaskContext>> CreateTaskToUpdateUniformData();
	protected:
		Camera(const Settings& settings, const std::shared_ptr<Node>& node);
		virtual void FillData(const std::shared_ptr<Core::GlobalContext>& globalContext, Data& data) = 0;
	protected:
		Settings _baseSettings;
		std::shared_ptr<Node> _node;

		std::shared_ptr<UniformBuffer<Data>> _uniformDataBuffer;
		std::shared_ptr<ContextualTask<UniformBuffer<Data>::WriteTaskContext>> _uniformDataWriteTask;
	};

	class PerspectiveCamera : public Camera
	{
	public:
		struct PerspectiveSettings final
		{
			std::float_t fov = Core::Math::kPi / 3;
		};
		struct Settings final
		{
			PerspectiveSettings perspective;
			Camera::Settings base;
		};
	protected:
		PerspectiveCamera(const Settings& settings, const std::shared_ptr<Node>& node);
	protected:
		PerspectiveSettings _perspectiveSettings;
	};

	class OrthographicCamera : public Camera
	{
	public:
		struct OrthographicSettings final
		{
			Vector2Float size {5.0f, 5.0f};
		};
		struct Settings final
		{
			OrthographicSettings orthographic;
			Camera::Settings base;
		};
	protected:
		OrthographicCamera(const Settings& settings, const std::shared_ptr<Node>& node);
	protected:
		OrthographicSettings _orthographicSettings;
	};
}
