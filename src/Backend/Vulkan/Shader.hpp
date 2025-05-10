#pragma once
#include <Core/Shader.hpp>

namespace MMPEngine::Backend::Vulkan
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

	class VertexShader final : public Core::VertexShader, public Shader
	{
	public:
		explicit VertexShader(std::filesystem::path&& path);
	};

	class PixelShader final : public Core::PixelShader, public Shader
	{
	public:
		explicit PixelShader(std::filesystem::path&& path);
	};

}