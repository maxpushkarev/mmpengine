#include <Frontend/Material.hpp>

namespace MMPEngine::Frontend
{
	ComputeMaterial::ComputeMaterial(const std::shared_ptr<Core::GlobalContext>& globalContext, Core::BaseMaterial::Parameters&& params, const std::shared_ptr<Core::ComputeShader>& computeShader)
		: Material<MMPEngine::Core::ComputeMaterial>(globalContext, std::move(params), computeShader)
	{
	}

	MeshMaterial::MeshMaterial(
		const std::shared_ptr<Core::GlobalContext>& globalContext,
		const Core::MeshMaterial::Settings& settings,
		Core::BaseMaterial::Parameters&& params,
		const std::shared_ptr<Core::VertexShader>& vs, 
		const std::shared_ptr<Core::PixelShader>& ps)
			: Material<MMPEngine::Core::MeshMaterial>(globalContext, settings, std::move(params), vs, ps)
	{
		
	}

}