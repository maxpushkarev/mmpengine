#include <Backend/Vulkan/Shader.hpp>
#include <cassert>

namespace MMPEngine::Backend::Vulkan
{
	Shader::Shader(std::filesystem::path&& path) : Core::Shader(std::move(path))
	{
		const auto shaderFileExtension = _path.extension();
		assert(shaderFileExtension.string() == ".spv");
	}

	ComputeShader::ComputeShader(std::filesystem::path&& path) : Core::Shader(std::move(path)), Core::ComputeShader({}), Shader({})
	{
	}

	VertexShader::VertexShader(std::filesystem::path&& path) : Core::Shader(std::move(path)), Core::VertexShader({}), Shader({})
	{
	}

	PixelShader::PixelShader(std::filesystem::path&& path) : Core::Shader(std::move(path)), Core::PixelShader({}), Shader({})
	{
	}
}
