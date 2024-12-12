#include <Core/Shader.hpp>
#include <cassert>
#include <fstream>

namespace MMPEngine::Core
{
	Shader::Shader(std::filesystem::path&& path) : _path(std::move(path))
	{
	}

	const void* Shader::GetCompiledBinaryData() const
	{
		return _compiledBinaryData.data();
	}

	std::size_t Shader::GetCompiledBinaryLength() const
	{
		return _compiledBinaryData.size();
	}

	std::shared_ptr<BaseTask> Shader::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->shader = shared_from_this();
		return std::make_shared<LoadCompiledShaderFile>(ctx);
	}


	Shader::LoadCompiledShaderFile::LoadCompiledShaderFile(const std::shared_ptr<InitTaskContext>& ctx) : ContextualTask<MMPEngine::Core::Shader::InitTaskContext>(ctx)
	{
	}

	void Shader::LoadCompiledShaderFile::Run(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::Run(stream);

		if (const auto shader = GetTaskContext()->shader)
		{
			assert(std::filesystem::exists(shader->_path));

			std::ifstream fileStream {shader->_path, std::ios::binary};

			shader->_compiledBinaryData = {
				(std::istreambuf_iterator<char>(fileStream)),
				(std::istreambuf_iterator<char>())
			};

			fileStream.close();
		}
	}


	void Shader::LoadCompiledShaderFile::OnComplete(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::OnComplete(stream);

		if(const auto shader = GetTaskContext()->shader)
		{
			std::filesystem::path p {};
			shader->_path.swap(p);
		}
	}

	ComputeShader::ComputeShader(std::filesystem::path&& path) : Shader(std::move(path))
	{
	}

	VertexShader::VertexShader(std::filesystem::path&& path) : Shader(std::move(path))
	{
	}

	PixelShader::PixelShader(std::filesystem::path&& path) : Shader(std::move(path))
	{
	}

}