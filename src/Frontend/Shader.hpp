#pragma once
#include <Core/Shader.hpp>
#include <Core/Context.hpp>
#include <filesystem>

#if defined (MMPENGINE_BACKEND_DX12) || defined(MMPENGINE_BACKEND_VULKAN)
#include <Backend/Shared/Shader.hpp>
#endif

namespace MMPEngine::Frontend
{
	class ShaderPack final : public Core::ShaderPack
	{
	public:
		ShaderPack(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::string& text);
		ShaderPack(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::filesystem::path& path);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::Shader> Unpack(std::string_view id) const override;
	private:
		std::shared_ptr<Core::ShaderPack> _impl;
	};
}