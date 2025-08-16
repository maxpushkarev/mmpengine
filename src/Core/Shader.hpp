#pragma once
#include <filesystem>
#include <vector>
#include <Core/Task.hpp>
#include <Core/Passkey.hpp>

namespace MMPEngine::Core
{
	class ShaderPack;

	class Shader : public IInitializationTaskSource, public std::enable_shared_from_this<Shader>
	{
	public:
		using PassControl = Core::PassControl<true, Core::ShaderPack>;
		struct Info final
		{
			enum class Type : std::uint8_t
			{
				Compute,
				Vertex,
				Pixel
			};

			std::string id;
			Type type;
			std::string entryPointName;
		};
	protected:
		explicit Shader(PassControl, Info&& settings);
	public:
		const void* GetCompiledBinaryData() const;
		std::size_t GetCompiledBinaryLength() const;
		const Info& GetInfo() const;
	protected:
		Info _info;
		std::vector<char> _binaryData;
	};

	class ShaderPack : public IInitializationTaskSource, public std::enable_shared_from_this<ShaderPack>
	{
	public:
		virtual std::shared_ptr<Shader> Unpack(std::string_view id) const = 0;
	};
}
