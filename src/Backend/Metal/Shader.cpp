#include <Backend/Metal/Shader.hpp>

namespace MMPEngine::Backend::Metal
{
    LibShader::LibShader(PassControl pass, const std::shared_ptr<LibShaderPack>& pack, Info&& settings) : Core::Shader(pass, std::move(settings)), _pack(pack)
    {
    }
    std::shared_ptr<Core::BaseTask> LibShader::CreateInitializationTask()
    {
        const auto ctx = std::make_shared<InitTaskContext>();
        ctx->entity = std::dynamic_pointer_cast<LibShader>(shared_from_this());
        return std::make_shared<InitTask>(ctx);
    }

    LibShader::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task<MMPEngine::Backend::Metal::LibShader::InitTaskContext>(ctx)
    {
    }

    void LibShader::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);
    }

    LibShaderPack::LibShaderPack(Settings&& settings) : _settings(std::move(settings))
    {
        for (std::size_t i = 0; i < _settings.libDataCollection.size(); ++i)
        {
            const auto& ld  = _settings.libDataCollection[i];
            assert(_id2IndexMap.find(ld.info.id) == _id2IndexMap.cend());
            _id2IndexMap[ld.info.id] = i;
        }
    }

    std::shared_ptr<Core::BaseTask> LibShaderPack::CreateInitializationTask()
    {
        return Core::BaseTask::kEmpty;
    }

    std::shared_ptr<Core::Shader> LibShaderPack::Unpack(std::string_view id) const
    {
        const auto idx = _id2IndexMap.at(id);
        auto ld = _settings.libDataCollection[idx];
        
        return std::make_shared<LibShader>(Core::PassKey {this}, std::dynamic_pointer_cast<LibShaderPack>(std::const_pointer_cast<Core::ShaderPack>(shared_from_this())), std::move(ld.info));
    }
}
