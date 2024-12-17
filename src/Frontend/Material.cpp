#include <Frontend/Material.hpp>

namespace MMPEngine::Frontend
{
	ComputeMaterial::ComputeMaterial(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::shared_ptr<Core::ComputeShader>& computeShader)
		: Material<MMPEngine::Core::ComputeMaterial>(globalContext, computeShader)
	{
	}
}