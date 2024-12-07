#pragma once
#include <Core/Shader.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Shader : public virtual Core::Shader
	{
	protected:
		explicit Shader(std::filesystem::path&& path);
	};

	class ComputeShader final : public Core::ComputeShader, public Shader
	{
	public:
		explicit ComputeShader(std::filesystem::path&& path);
	};
}