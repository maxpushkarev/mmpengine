#include "Camera.hpp"
#include <Core/Camera.hpp>

namespace MMPEngine::Core
{
	Camera::Camera(const Settings& settings, const std::shared_ptr<Node>& node) : BaseEntity(settings.name), _baseSettings(settings), _node(node)
	{
	}

	std::shared_ptr<Node> Camera::GetNode() const
	{
		return _node;
	}

	std::shared_ptr<BaseEntity> Camera::GetUniformDataEntity() const
	{
		return _uniformDataBuffer;
	}

    std::shared_ptr<ContextualTask<Camera::UpdateDataTaskContext>> Camera::CreateTaskToUpdateUniformData()
	{
		const auto ctx = std::make_shared<InternalUpdateDataTaskContext>();
		ctx->camera = std::dynamic_pointer_cast<Camera>(shared_from_this());
		return std::make_shared<InternalUpdateDataTask>(ctx);
	}

	Camera::InternalUpdateDataTask::InternalUpdateDataTask(const std::shared_ptr<InternalUpdateDataTaskContext>& ctx) : ContextualTask<MMPEngine::Core::Camera::UpdateDataTaskContext>(ctx), _internalContext(ctx)
	{
	}

	void Camera::InternalUpdateDataTask::OnScheduled(const std::shared_ptr<BaseStream>& stream)
	{
		ContextualTask::OnScheduled(stream);

		const auto camera = _internalContext->camera;
		const auto precomputed = _internalContext->precomputed;

		if(precomputed.has_value())
		{
			camera->_uniformDataWriteTask->GetTaskContext()->data = precomputed.value();
		}
		else
		{
			camera->FillData(stream->GetGlobalContext(), camera->_uniformDataWriteTask->GetTaskContext()->data);
		}

		stream->Schedule(camera->_uniformDataWriteTask);
	}


	PerspectiveCamera::PerspectiveCamera(const Settings& settings, const std::shared_ptr<Node>& node) : Camera(settings.base, node), _perspectiveSettings(settings.perspective)
	{
	}

	OrthographicCamera::OrthographicCamera(const Settings& settings, const std::shared_ptr<Node>& node) : Camera(settings.base, node), _orthographicSettings(settings.orthographic)
	{
	}
}