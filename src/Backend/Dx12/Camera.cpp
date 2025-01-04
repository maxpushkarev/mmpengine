#include <DirectXMath.h>
#include <Backend/Dx12/Camera.hpp>
#include <Backend/Dx12/Buffer.hpp>

namespace MMPEngine::Backend::Dx12
{
	Camera::Camera() = default;
	Camera::~Camera() = default;

	Camera::InitTask::InitTask(const std::shared_ptr<CameraTaskContext>& ctx) : Task(ctx)
	{
	}

	void Camera::FillNonProjectionData(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::shared_ptr<Core::Node>& node, Core::Camera::Data& data)
	{
		Core::Matrix4x4 l2w {};
		globalContext->math->CalculateLocalToWorldSpaceMatrix(l2w, node);
		globalContext->math->GetColumn(data.worldPosition, 3, l2w);

		Core::Vector4Float cameraFwd {};
		Core::Vector4Float cameraUp {};

		globalContext->math->GetColumn(cameraFwd, 2, l2w);
		globalContext->math->GetColumn(cameraUp, 1, l2w);

		const auto camFwdVec = DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&cameraFwd));
		const auto camUpVec = DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&cameraUp));
		const auto camPosVec = DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&data.worldPosition));

		const auto view = DirectX::XMMatrixLookToLH(camPosVec, camFwdVec, camUpVec);
		XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&data.viewMatrix), DirectX::XMMatrixTranspose(view));
	}

	void Camera::InitTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
		const auto camera = GetTaskContext()->entity;

		stream->Schedule(std::make_shared<Core::FunctionalTask>(
			[camera](const auto& s)
			{
				camera->GetUniformBufferRef() = std::make_shared<UniformBuffer<Core::Camera::Data>>();
				s->Schedule(camera->GetUniformBufferRef()->CreateInitializationTask());
			},
			Core::FunctionalTask::Handler{},
			Core::FunctionalTask::Handler{}
		));

		stream->Schedule(std::make_shared<Core::FunctionalTask>(
			Core::FunctionalTask::Handler{},
			[camera](const auto& s)
			{
				Core::Camera::Data data {};
				camera->FillDataInternal(s->GetGlobalContext(), data);
				camera->GetUniformBufferUpdateTaskRef() = camera->GetUniformBufferRef()->CreateWriteAsyncTask(data);
			},
			Core::FunctionalTask::Handler{}
		));
	}

	PerspectiveCamera::PerspectiveCamera(const Settings& settings, const std::shared_ptr<Core::Node>& node, const Target& target) : Core::PerspectiveCamera(settings, node, target)
	{
	}

	std::shared_ptr<Core::BaseTask> PerspectiveCamera::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<CameraTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<Dx12::Camera>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	void PerspectiveCamera::FillData(const std::shared_ptr<Core::GlobalContext>& globalContext, Data& data)
	{
		FillNonProjectionData(globalContext, _node, data);

		const auto size = _target.color.front().tex->GetSettings().base.size;
		const auto aspect = static_cast<std::float_t>(size.x) / static_cast<std::float_t>(size.y);

		const auto nearPlane = _baseSettings.nearPlane;
		const auto farPlane = _baseSettings.farPlane;
		const auto proj = DirectX::XMMatrixPerspectiveFovLH(_perspectiveSettings.fov, aspect, nearPlane, farPlane);
		XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&data.projMatrix), DirectX::XMMatrixTranspose(proj));
	}

	std::shared_ptr<Core::UniformBuffer<Core::Camera::Data>>& PerspectiveCamera::GetUniformBufferRef()
	{
		return _uniformDataBuffer;
	}

	std::shared_ptr<Core::ContextualTask<Core::UniformBuffer<Core::Camera::Data>::WriteTaskContext>>& PerspectiveCamera::GetUniformBufferUpdateTaskRef()
	{
		return  _uniformDataWriteTask;
	}

	void PerspectiveCamera::FillDataInternal(const std::shared_ptr<Core::GlobalContext>& globalContext, Core::Camera::Data& data)
	{
		FillData(globalContext, data);
	}

	OrthographicCamera::OrthographicCamera(const Settings& settings, const std::shared_ptr<Core::Node>& node, const Target& target) : Core::OrthographicCamera(settings, node, target)
	{
	}

	std::shared_ptr<Core::BaseTask> OrthographicCamera::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<CameraTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<Dx12::Camera>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	void OrthographicCamera::FillData(const std::shared_ptr<Core::GlobalContext>& globalContext, Data& data)
	{
		FillNonProjectionData(globalContext, _node, data);

		const auto proj = DirectX::XMMatrixOrthographicLH(
			_orthographicSettings.size.x,
			_orthographicSettings.size.y,
			_baseSettings.nearPlane,
			_baseSettings.farPlane
		);

		XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&data.projMatrix), DirectX::XMMatrixTranspose(proj));
	}

	std::shared_ptr<Core::UniformBuffer<Core::Camera::Data>>& OrthographicCamera::GetUniformBufferRef()
	{
		return _uniformDataBuffer;
	}

	std::shared_ptr<Core::ContextualTask<Core::UniformBuffer<Core::Camera::Data>::WriteTaskContext>>& OrthographicCamera::GetUniformBufferUpdateTaskRef()
	{
		return _uniformDataWriteTask;
	}

	void OrthographicCamera::FillDataInternal(const std::shared_ptr<Core::GlobalContext>& globalContext, Core::Camera::Data& data)
	{
		FillData(globalContext, data);
	}

}