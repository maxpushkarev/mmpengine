#include <Core/Camera.hpp>
#include <cassert>
#include <Core/Material.hpp>

namespace MMPEngine::Core
{
	Camera::Camera(const Settings& settings, const std::shared_ptr<Node>& node, const Target& target) : BaseEntity(settings.name), _target(target), _baseSettings(settings), _node(node)
	{
		assert(!_target.color.empty());
		assert(_target.color.size() <= Core::MeshMaterial::Settings::Blend::kMaxRenderTargets);

		const auto& firstColorTarget = *_target.color.cbegin();
		assert(std::all_of(_target.color.cbegin(), _target.color.cend(), [&firstColorTarget](const auto& t)
		{
			return (t.tex->GetSettings().base.antialiasing == firstColorTarget.tex->GetSettings().base.antialiasing) && (t.tex->GetSettings().base.size == firstColorTarget.tex->GetSettings().base.size);
		}));

		assert((firstColorTarget.tex->GetSettings().base.antialiasing == _target.depthStencil.tex->GetSettings().base.antialiasing) && (firstColorTarget.tex->GetSettings().base.size == _target.depthStencil.tex->GetSettings().base.size));
	}

	std::shared_ptr<const Node> Camera::GetNode() const
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


	PerspectiveCamera::PerspectiveCamera(const Settings& settings, const std::shared_ptr<Node>& node, const Target& target) : Camera(settings.base, node, target), _perspectiveSettings(settings.perspective)
	{
	}

	OrthographicCamera::OrthographicCamera(const Settings& settings, const std::shared_ptr<Node>& node, const Target& target) : Camera(settings.base, node, target), _orthographicSettings(settings.orthographic)
	{
	}
}
