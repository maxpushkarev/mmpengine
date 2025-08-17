#pragma once
#include <Core/Shader.hpp>
#include <Core/Entity.hpp>
#include <filesystem>
#include <vector>

namespace MMPEngine::Backend::Shared
{
	class ByteCodeFileShader final : public Core::Shader
	{
	public:
		ByteCodeFileShader(PassControl, Info&& settings, std::filesystem::path&& path);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	private:
		class InitTaskContext final : public Core::EntityTaskContext<ByteCodeFileShader>
		{
		};
		class InitTask final : public Core::ContextualTask<InitTaskContext>
		{
		public:
			InitTask(const std::shared_ptr<InitTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			void OnComplete(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	private:
		std::filesystem::path _path;
	};

	class ByteCodeFileShaderPack final : public Core::ShaderPack
	{
	public:
		struct Settings final
		{
			struct PreloadData final
			{
				Core::Shader::Info info;
				std::filesystem::path path;
			};

			std::vector<PreloadData> preloadDataCollection;
		};
		ByteCodeFileShaderPack(Settings&& settings);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::Shader> Unpack(std::string_view id) const override;
	private:
		Settings _settings;
		std::unordered_map<std::string_view, std::size_t> _id2PreloadIndexMap;
	};
}
