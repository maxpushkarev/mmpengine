#include <Backend/Shared/Shader.hpp>
#include <cassert>
#include <fstream>

namespace MMPEngine::Backend::Shared
{
	ByteCodeFileShader::ByteCodeFileShader(PassControl pass, Info&& settings, std::filesystem::path&& path) : Core::Shader(pass, std::move(settings)), _path(path)
	{
	}

	std::shared_ptr<Core::BaseTask> ByteCodeFileShader::CreateInitializationTask()
	{
		return Core::BaseTask::kEmpty;
	}

	ByteCodeFileShader::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : ContextualTask<MMPEngine::Backend::Shared::ByteCodeFileShader::InitTaskContext>(ctx)
	{
	}

	void ByteCodeFileShader::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		ContextualTask::Run(stream);

		if (const auto shader = GetTaskContext()->entity)
		{
			assert(std::filesystem::exists(shader->_path));

			std::ifstream fileStream{ shader->_path, std::ios::binary };

			shader->_binaryData = {
				(std::istreambuf_iterator<char>(fileStream)),
				(std::istreambuf_iterator<char>())
			};

			fileStream.close();
		}
	}

	void ByteCodeFileShader::InitTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		ContextualTask::OnComplete(stream);

		if (const auto shader = GetTaskContext()->entity)
		{
			std::filesystem::path p{};
			shader->_path.swap(p);
		}
	}

	ByteCodeFileShaderPack::ByteCodeFileShaderPack(Settings&& settings) : _settings(std::move(settings))
	{
		for (std::size_t i = 0; i < _settings.preloadDataCollection.size(); ++i)
		{
			const auto& pd  = _settings.preloadDataCollection[i];
			assert(_id2PreloadIndexMap.find(pd.info.id) == _id2PreloadIndexMap.cend());
			_id2PreloadIndexMap[pd.info.id] = i;
		}
	}

	std::shared_ptr<Core::BaseTask> ByteCodeFileShaderPack::CreateInitializationTask()
	{
		return Core::BaseTask::kEmpty;
	}

	std::shared_ptr<Core::Shader> ByteCodeFileShaderPack::Unpack(std::string_view id) const
	{
		const auto idx = _id2PreloadIndexMap.at(id);
		auto pd = _settings.preloadDataCollection[idx];
		
		return std::make_shared<ByteCodeFileShader>(Core::PassKey {this}, std::move(pd.info), std::move(pd.path));
	}
}