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

		nlohmann::json Load(const std::filesystem::path& path)
		{
			assert(path.extension().string() == ".json");
			assert(std::filesystem::exists(path));

			std::ifstream fileStream(path);
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
						std::filesystem::path { std::string { byteCodeFileNode["path"] } }
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
		}
	}

	ShaderPack::ShaderPack(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::string& text)
	{
		Parse(globalContext, nlohmann::json::parse(text),_impl);
	}

	ShaderPack::ShaderPack(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::filesystem::path& path)
	{
		Parse(globalContext, Load(path), _impl);
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
