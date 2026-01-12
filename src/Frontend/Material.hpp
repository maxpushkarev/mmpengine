#pragma once
#include <Core/Material.hpp>

namespace MMPEngine::Frontend
{
    class MeshMaterial final : public Core::MeshMaterial
    {
    public:
        MeshMaterial(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings, Parameters&& params, const std::shared_ptr<Core::Shader>& vs, const std::shared_ptr<Core::Shader>& ps);
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
    private:
        std::shared_ptr<Core::MeshMaterial> _impl;
    };

    class ComputeMaterial final : public Core::ComputeMaterial
    {
    public:
        ComputeMaterial(const std::shared_ptr<Core::GlobalContext>& globalContext, Parameters&& params, const std::shared_ptr<Core::Shader>& сs);
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
    private:
        std::shared_ptr<Core::ComputeMaterial> _impl;
    };
}
