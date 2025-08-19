#include <Frontend/Shader.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <cassert>

namespace MMPEngine::Frontend
{
	namespace 
	{
		Core::Shader::Info::Type GetType(std::string_view str)
		{
			if (str == "Compute")
			{
				return Core::Shader::Info::Type::Compute;
			}

			if (str == "Vertex")
			{
				return Core::Shader::Info::Type::Vertex;
			}

			if (str == "Pixel")
			{
				return Core::Shader::Info::Type::Pixel;
			}

			throw Core::UnsupportedException("unknown shader type");
		}

		nlohmann::json Load(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::filesystem::path& path)
		{
            const auto fullPath = globalContext->environment.baseExecutablePath / path;
			assert(fullPath.extension().string() == ".json");
			assert(std::filesystem::exists(fullPath));

			std::ifstream fileStream(fullPath);
			const nlohmann::json data = nlohmann::json::parse(fileStream);
			fileStream.close();
			return data;
		}

		void Parse(const std::shared_ptr<Core::GlobalContext>& globalContext, const nlohmann::json& root, std::shared_ptr<Core::ShaderPack>& impl)
		{
			const auto& shaderPackNode = root["shaderPack"];

			std::string backendStr{};

			switch (globalContext->settings.backend)
			{
			case Core::BackendType::Dx12:
				backendStr = "Dx12";
				break;
			case Core::BackendType::Vulkan:
				backendStr = "Vulkan";
				break;
			case Core::BackendType::Metal:
				backendStr = "Metal";
				break;
			}

			const auto& backendNode = shaderPackNode[backendStr];

#if defined (MMPENGINE_BACKEND_DX12) || defined(MMPENGINE_BACKEND_VULKAN)
			if (backendNode.contains("byteCodeFiles"))
			{
				const auto& byteCodeFilesNode = backendNode["byteCodeFiles"];
				Backend::Shared::ByteCodeFileShaderPack::Settings byteCodeFilePackSettings{};

				for (const auto& byteCodeFileNode : byteCodeFilesNode) {
					byteCodeFilePackSettings.preloadDataCollection.push_back(Backend::Shared::ByteCodeFileShaderPack::Settings::PreloadData{
						Core::Shader::Info {
							std::string { byteCodeFileNode["id"] },
							GetType(std::string { byteCodeFileNode["type"] }),
							std::string { byteCodeFileNode["entryPoint"] }
						},
                        globalContext->baseExecutablePath / std::filesystem::path { std::string { byteCodeFileNode["path"] } }
						});
				}

				impl = std::make_shared<Backend::Shared::ByteCodeFileShaderPack>(std::move(byteCodeFilePackSettings));
				return;
			}

#if defined (MMPENGINE_BACKEND_DX12)
			throw Core::UnsupportedException("unable to create shader pack for DX12 backend");
#endif

#if defined (MMPENGINE_BACKEND_VULKAN)
			throw Core::UnsupportedException("unable to create shader pack for Vulkan backend");
#endif

#endif
            
#ifdef MMPENGINE_BACKEND_METAL
            if (backendNode.contains("lib"))
            {
                const auto& libNode = backendNode["lib"];
                Backend::Metal::LibShaderPack::Settings libPackSettings{};

                if(libNode.contains("shaders"))
                {
                    const auto& shadersNode = libNode["shaders"];
                    for (const auto& shaderNode : shadersNode) {
                        libPackSettings.libDataCollection.push_back(Backend::Metal::LibShaderPack::Settings::LibData {
                            Core::Shader::Info {
                                std::string { shaderNode["id"] },
                                GetType(std::string { shaderNode["type"] }),
                                std::string { shaderNode["entryPoint"] }
                            }
                        });
                    }
                }
                
                std::vector<char> rawData {};

                if(libNode.contains("path"))
                {
                    const auto pathNode = libNode["path"];
                    const auto pathNodeStr = std::string {pathNode};
                    const auto basePath = globalContext->environment.baseExecutablePath;
                    const auto path = basePath / std::filesystem::path(pathNodeStr);
                    
                    assert(std::filesystem::exists(path));

                    std::ifstream fileStream{ path, std::ios::binary };

                    rawData = {
                        (std::istreambuf_iterator<char>(fileStream)),
                        (std::istreambuf_iterator<char>())
                    };

                    fileStream.close();
                }
                
                impl = std::make_shared<Backend::Metal::LibShaderPack>(std::move(libPackSettings), std::move(rawData));
                return;
            }

            throw Core::UnsupportedException("unable to create shader pack for Metal backend");
#endif
		}
	}

	ShaderPack::ShaderPack(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::string& text)
	{
		Parse(globalContext, nlohmann::json::parse(text),_impl);
	}

	ShaderPack::ShaderPack(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::filesystem::path& path)
	{
		Parse(globalContext, Load(globalContext, path), _impl);
	}

	std::shared_ptr<Core::BaseTask> ShaderPack::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();
	}

	std::shared_ptr<Core::Shader> ShaderPack::Unpack(std::string_view id) const
	{
		return _impl->Unpack(id);
	}
}
