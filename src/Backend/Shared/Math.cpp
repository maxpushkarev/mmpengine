#include <Backend/Shared/Math.hpp>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

namespace MMPEngine::Backend::Shared
{
	std::float_t GLMMath::Dot(const Core::Vector3Float& v1, const Core::Vector3Float& v2) const
	{
		return glm::dot(reinterpret_cast<const glm::vec3&>(v1), reinterpret_cast<const glm::vec3&>(v2));
	}

}
