#include <Frontend/Shader.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

namespace MMPEngine::Frontend
{
	template<>
	std::shared_ptr<Core::ComputeShader> Shader::LoadFromFile(const std::shared_ptr<Core::GlobalContext>& globalContext, std::filesystem::path&& path)
	{
		auto specificShaderPath = GetSpecificPath(globalContext, std::move(path));

		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::ComputeShader>(std::move(specificShaderPath));
#else
			throw Core::UnsupportedException("unable to load compute shader for DX12 backend");
#endif
		}

		return nullptr;
	}


	template<>
	std::shared_ptr<Core::VertexShader> Shader::LoadFromFile(const std::shared_ptr<Core::GlobalContext>& globalContext, std::filesystem::path&& path)
	{
		auto specificShaderPath = GetSpecificPath(globalContext, std::move(path));

		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::VertexShader>(std::move(specificShaderPath));
#else
			throw Core::UnsupportedException("unable to load vertex shader for DX12 backend");
#endif
		}

		return nullptr;
	}

	template<>
	std::shared_ptr<Core::PixelShader> Shader::LoadFromFile(const std::shared_ptr<Core::GlobalContext>& globalContext, std::filesystem::path&& path)
	{
		auto specificShaderPath = GetSpecificPath(globalContext, std::move(path));

		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::PixelShader>(std::move(specificShaderPath));
#else
			throw Core::UnsupportedException("unable to load pixel shader for DX12 backend");
#endif
		}

		return nullptr;
	}


	std::filesystem::path Shader::GetSpecificPath(const std::shared_ptr<Core::GlobalContext>& globalContext, std::filesystem::path&& path)
	{
		assert(path.extension().string() == ".json");
		assert(std::filesystem::exists(path));

		std::ifstream fileStream(path);
		const nlohmann::json data = nlohmann::json::parse(fileStream);
		fileStream.close();

		const auto& shaderCfg = data["shaderConfig"];
		std::string backendStr {};

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

		const auto& specificShaderPathObj = shaderCfg[backendStr]["path"];
		const std::string str = specificShaderPathObj;
		return std::filesystem::path {str};
	}
}
