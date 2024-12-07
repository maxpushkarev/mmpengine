#pragma once
#include <filesystem>
#include <vector>
#include <Core/Task.hpp>

namespace MMPEngine::Core
{
	class Shader : public IInitializationTaskSource, public std::enable_shared_from_this<Shader>
	{
	protected:

		class InitTaskContext : public TaskContext
		{
		public:
			std::weak_ptr<Shader> shader;
		};

		class LoadCompiledShaderFile : public TaskWithInternalContext<InitTaskContext>
		{
		public:
			LoadCompiledShaderFile(const std::shared_ptr<InitTaskContext>& ctx);
			void Run(const std::shared_ptr<BaseStream>& stream) override;
			void OnComplete(const std::shared_ptr<BaseStream>& stream) override;
		};

		explicit Shader(std::filesystem::path&& path);
	public:
		const void* GetCompiledBinaryData() const;
		std::size_t GetCompiledBinaryLength() const;
		std::shared_ptr<BaseTask> CreateInitializationTask() override;
	protected:
		std::vector<char> _compiledBinaryData;
		std::filesystem::path _path;
	};

	class ComputeShader : public virtual Shader
	{
	protected:
		explicit ComputeShader(std::filesystem::path&& path);
	};

	class VertexShader : public virtual Shader
	{
	protected:
		explicit VertexShader(std::filesystem::path&& path);
	};

	class PixelShader : public virtual Shader
	{
	protected:
		explicit PixelShader(std::filesystem::path&& path);
	};

}
