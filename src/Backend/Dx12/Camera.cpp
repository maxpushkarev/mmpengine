#include <Backend/Dx12/Camera.hpp>
#include <Backend/Dx12/Buffer.hpp>

namespace MMPEngine::Backend::Dx12
{
	Camera::InitTask::InitTask(const std::shared_ptr<CameraTaskContext>& ctx) : Task(ctx)
	{
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

	PerspectiveCamera::PerspectiveCamera(const Settings& settings, const std::shared_ptr<Core::Node>& node) : Core::PerspectiveCamera(settings, node)
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

	OrthographicCamera::OrthographicCamera(const Settings& settings, const std::shared_ptr<Core::Node>& node) : Core::OrthographicCamera(settings, node)
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