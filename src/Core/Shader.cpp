#include <Core/Shader.hpp>

namespace MMPEngine::Core
{
	Shader::Shader(PassControl, Info&& settings) : _info(std::move(settings))
	{
	}

	const void* Shader::GetCompiledBinaryData() const
	{
		return _binaryData.data();
	}

	std::size_t Shader::GetCompiledBinaryLength() const
	{
		return _binaryData.size();
	}

	const Shader::Info& Shader::GetInfo() const
	{
		return _info;
	}
}