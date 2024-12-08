#include <Frontend/Material.hpp>

namespace MMPEngine::Frontend
{
	ComputeMaterial::ComputeMaterial(const std::shared_ptr<Core::AppContext>& appContext, const std::shared_ptr<Core::ComputeShader>& computeShader)
		: Material<MMPEngine::Core::ComputeMaterial>(appContext, computeShader)
	{
	}
}