#include <Core/Shader.hpp>
#include <Core/Entity.hpp>
#include <Backend/Metal/Task.hpp>
#include <Metal/Metal.hpp>

namespace MMPEngine::Backend::Metal
{
    class LibShaderPack;

    class LibShader final : public Core::Shader
    {
    public:
        LibShader(PassControl, const std::shared_ptr<LibShaderPack>& pack, Info&& settings);
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
        MTL::Function* GetNativeFunction() const;
    private:
        class InitTaskContext final : public Core::EntityTaskContext<LibShader>
        {
        };
        class InitTask final : public Task<InitTaskContext>
        {
        public:
            InitTask(const std::shared_ptr<InitTaskContext>& ctx);
        protected:
            void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
        };
        std::shared_ptr<LibShaderPack> _pack;
        NS::SharedPtr<MTL::Function> _nativeFunction = nullptr;
    };

    class LibShaderPack final : public Core::ShaderPack
    {
    public:
        struct Settings final
        {
            struct LibData final
            {
                Core::Shader::Info info;
            };

            std::vector<LibData> libDataCollection;
        };
        LibShaderPack(Settings&& settings, std::vector<char>&& rawData);
        LibShaderPack(const LibShaderPack&) = delete;
        LibShaderPack(LibShaderPack&&) = delete;
        LibShaderPack& operator=(const LibShaderPack&) = delete;
        LibShaderPack& operator=(LibShaderPack&&) = delete;
        ~LibShaderPack() override;
        MTL::Library* GetNativeLibraryPtr() const;
        std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
        std::shared_ptr<Core::Shader> Unpack(std::string_view id) const override;
    private:
        
        class InitTaskContext final : public Core::EntityTaskContext<LibShaderPack>
        {
        };
        class InitTask final : public Task<InitTaskContext>
        {
        public:
            InitTask(const std::shared_ptr<InitTaskContext>& ctx);
        protected:
            void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
            void OnComplete(const std::shared_ptr<Core::BaseStream>& stream) override;
        };
        
        Settings _settings;
        std::vector<char> _rawData;
        dispatch_data_t _dispatchData = nullptr;
        NS::SharedPtr<MTL::Library> _nativeLibrary = nullptr;
        std::unordered_map<std::string_view, std::size_t> _id2IndexMap;
    };
}
