#include <Backend/Metal/Camera.hpp>
#include <Backend/Metal/Buffer.hpp>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.inl>
#include <glm/ext/matrix_transform.hpp>

namespace MMPEngine::Backend::Metal
{
    Camera::Camera() = default;
    Camera::~Camera() = default;

    Camera::InitTask::InitTask(const std::shared_ptr<CameraTaskContext>& ctx) : Task(ctx)
    {
    }

    void Camera::FillNonProjectionData(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::shared_ptr<Core::Node>& node, Core::Camera::Data& data)
    {
        Core::Matrix4x4 l2w{};
        globalContext->math->CalculateLocalToWorldSpaceMatrix(l2w, node);
        globalContext->math->GetColumn(data.worldPosition, 3, l2w);

        Core::Vector4Float cameraFwd{};
        Core::Vector4Float cameraUp{};

        globalContext->math->GetColumn(cameraFwd, 2, l2w);
        globalContext->math->GetColumn(cameraUp, 1, l2w);


        const auto camFwdVec = reinterpret_cast<const glm::vec3*>(&cameraFwd);
        const auto camUpVec = reinterpret_cast<const glm::vec3*>(&cameraUp);
        const auto camPosVec = reinterpret_cast<const glm::vec3*>(&data.worldPosition);

        const auto center = *camPosVec + *camFwdVec;

        glm::mat4 viewMat = glm::transpose(glm::lookAtLH(*camPosVec, center, *camUpVec));
        std::memcpy(&data.viewMatrix, &viewMat, sizeof(data.viewMatrix));
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
                Core::Camera::Data data{};
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
        ctx->entity = std::dynamic_pointer_cast<Metal::Camera>(shared_from_this());
        return std::make_shared<InitTask>(ctx);
    }

    void PerspectiveCamera::FillData(const std::shared_ptr<Core::GlobalContext>& globalContext, Data& data)
    {
        FillNonProjectionData(globalContext, _node, data);

        const auto size = _target.color.front().tex->GetSettings().base.size;
        const auto nearPlane = _baseSettings.nearPlane;
        const auto farPlane = _baseSettings.farPlane;

        const auto proj = glm::transpose(glm::perspectiveFovLH_NO(
            _perspectiveSettings.fov,
            static_cast<std::float_t>(size.x),
            static_cast<std::float_t>(size.y),
            nearPlane,
            farPlane
        ));

        std::memcpy(&data.projMatrix, &proj, sizeof(data.projMatrix));
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
        ctx->entity = std::dynamic_pointer_cast<Metal::Camera>(shared_from_this());
        return std::make_shared<InitTask>(ctx);
    }

    void OrthographicCamera::FillData(const std::shared_ptr<Core::GlobalContext>& globalContext, Data& data)
    {
        FillNonProjectionData(globalContext, _node, data);

        const auto proj = glm::transpose(glm::orthoLH_NO(
            -_orthographicSettings.size.x * 0.5f, _orthographicSettings.size.x * 0.5f,
            -_orthographicSettings.size.y * 0.5f, _orthographicSettings.size.y * 0.5f,
            _baseSettings.nearPlane,
            _baseSettings.farPlane
        ));

        std::memcpy(&data.projMatrix, &proj, sizeof(data.projMatrix));
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

