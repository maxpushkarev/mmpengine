#include <Backend/Metal/Shader.hpp>

namespace MMPEngine::Backend::Metal
{
    LibShader::LibShader(PassControl pass, const std::shared_ptr<LibShaderPack>& pack, Info&& settings) : Core::Shader(pass, std::move(settings)), _pack(pack)
    {
    }

    LibShader::~LibShader()
    {
        if(_nativeFunction)
        {
            _nativeFunction->release();
        }
    }

    MTL::Function* LibShader::GetNativeFunction() const
    {
        return _nativeFunction;
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
        
        const auto shader = GetTaskContext()->entity;
        const auto funcName = NS::String::string(shader->GetInfo().entryPointName.c_str(), NS::ASCIIStringEncoding);
        
        shader->_nativeFunction = shader->_pack->GetNativeLibraryPtr()->newFunction(funcName);
        
        funcName->release();
        
        assert(shader->_nativeFunction != nullptr);
    }

    LibShaderPack::LibShaderPack(Settings&& settings,std::vector<char>&& rawData) : _settings(std::move(settings)), _rawData(std::move(rawData))
    {
        for (std::size_t i = 0; i < _settings.libDataCollection.size(); ++i)
        {
            const auto& ld  = _settings.libDataCollection[i];
            assert(_id2IndexMap.find(ld.info.id) == _id2IndexMap.cend());
            _id2IndexMap[ld.info.id] = i;
        }
    }

    LibShaderPack::~LibShaderPack()
    {
        if(_nativeLibrary)
        {
            _nativeLibrary->release();
        }
        
        if(_dispatchData)
        {
            dispatch_release(_dispatchData);
        }
    }

    MTL::Library* LibShaderPack::GetNativeLibraryPtr() const
    {
        return _nativeLibrary;
    }

    std::shared_ptr<Core::Shader> LibShaderPack::Unpack(std::string_view id) const
    {
        const auto idx = _id2IndexMap.at(id);
        auto ld = _settings.libDataCollection[idx];
        
        return std::make_shared<LibShader>(Core::PassKey {this}, std::dynamic_pointer_cast<LibShaderPack>(std::const_pointer_cast<Core::ShaderPack>(shared_from_this())), std::move(ld.info));
    }

    std::shared_ptr<Core::BaseTask> LibShaderPack::CreateInitializationTask()
    {
        const auto ctx = std::make_shared<InitTaskContext>();
        ctx->entity = std::dynamic_pointer_cast<LibShaderPack>(shared_from_this());
        return std::make_shared<InitTask>(ctx);
    }

    LibShaderPack::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task<MMPEngine::Backend::Metal::LibShaderPack::InitTaskContext>(ctx)
    {
    }

    void LibShaderPack::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::Run(stream);
        
        const auto pack = GetTaskContext()->entity;
        
        pack->_dispatchData = dispatch_data_create(pack->_rawData.data(), pack->_rawData.size(), nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
        
        NS::Error* error = nullptr;
       
        pack->_nativeLibrary = _specificGlobalContext->device->GetNative()->newLibrary(pack->_dispatchData, &error);
        
        assert(error == nullptr);
    }

    void LibShaderPack::InitTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
    {
        Task::OnComplete(stream);
        
        const auto pack = GetTaskContext()->entity;
        
        std::vector<char> v;
        std::swap(pack->_rawData, v);
        
        if(pack->_dispatchData)
        {
            dispatch_release(pack->_dispatchData);
            pack->_dispatchData = nullptr;
        }
    }
}
