#include <Backend/Metal/Material.hpp>

namespace MMPEngine::Backend::Metal
{
    MeshMaterial::MeshMaterial(const Settings& settings, Parameters&& params, const std::shared_ptr<Core::Shader>& vs, const std::shared_ptr<Core::Shader>& ps)
        : Core::MeshMaterial(settings, std::move(params), vs, ps)
    {
    }

    std::shared_ptr<Core::BaseTask> MeshMaterial::CreateInitializationTask()
    {
        return Core::BaseTask::kEmpty;
    }

    ComputeMaterial::ComputeMaterial(Parameters&& params, const std::shared_ptr<Core::Shader>& cs)
        : Core::ComputeMaterial(std::move(params), cs)
    {
    }

    std::shared_ptr<Core::BaseTask> ComputeMaterial::CreateInitializationTask()
    {
        return Core::BaseTask::kEmpty;
    }
}
