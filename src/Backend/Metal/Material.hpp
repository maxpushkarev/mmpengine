#pragma once
#include <Core/Material.hpp>

namespace MMPEngine::Backend::Metal
{
    class MeshMaterial final : public Core::MeshMaterial
    {
    public:
        MeshMaterial(const Settings& settings, Parameters&& params, const std::shared_ptr<Core::Shader>& vs, const std::shared_ptr<Core::Shader>& ps);
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
    };

    class ComputeMaterial final : public Core::ComputeMaterial
    {
    public:
        ComputeMaterial(Parameters&& params, const std::shared_ptr<Core::Shader>& сs);
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
    };
}

