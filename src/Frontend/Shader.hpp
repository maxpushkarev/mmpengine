#pragma once
#include <Core/Shader.hpp>
#include <Core/Context.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Shader.hpp>
#endif

namespace MMPEngine::Frontend
{
	class Shader final
	{
	public:
		template<typename TCoreShader>
		static std::shared_ptr<TCoreShader> LoadFromFile(const std::shared_ptr<Core::AppContext>& appContext, std::filesystem::path&& path);
	private:
		static std::filesystem::path GetSpecificPath(const std::shared_ptr<Core::AppContext>& appContext, std::filesystem::path&& path);
	};

	template<>
	std::shared_ptr<Core::ComputeShader> Shader::LoadFromFile(const std::shared_ptr<Core::AppContext>& appContext, std::filesystem::path&& path);

	template<>
	std::shared_ptr<Core::VertexShader> Shader::LoadFromFile(const std::shared_ptr<Core::AppContext>& appContext, std::filesystem::path&& path);

	template<>
	std::shared_ptr<Core::PixelShader> Shader::LoadFromFile(const std::shared_ptr<Core::AppContext>& appContext, std::filesystem::path&& path);

	template<typename TCoreShader>
	std::shared_ptr<TCoreShader> Shader::LoadFromFile(const std::shared_ptr<Core::AppContext>& appContext, std::filesystem::path&& path)
	{
		return nullptr;
	}
}