#include <Backend/Shared/Math.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace MMPEngine::Backend::Shared
{
	std::float_t GLMMath::Dot(const Core::Vector3Float& v1, const Core::Vector3Float& v2) const
	{
		return glm::dot(reinterpret_cast<const glm::vec3&>(v1), reinterpret_cast<const glm::vec3&>(v2));
	}

	void GLMMath::Cross(Core::Vector3Float& res, const Core::Vector3Float& v1, const Core::Vector3Float& v2) const
	{
		const auto cross = glm::cross(reinterpret_cast<const glm::vec3&>(v1), reinterpret_cast<const glm::vec3&>(v2));
		std::memcpy(&res, &cross, sizeof(res));
	}

	std::float_t GLMMath::Magnitude(const Core::Vector3Float& v) const
	{
		return glm::length(reinterpret_cast<const glm::vec3&>(v));
	}

	std::float_t GLMMath::SquaredMagnitude(const Core::Vector3Float& v) const
	{
		return glm::length2(reinterpret_cast<const glm::vec3&>(v));
	}

	void GLMMath::Normalize(Core::Vector3Float& v) const
	{
		const auto nrm = glm::normalize(reinterpret_cast<const glm::vec3&>(v));
		std::memcpy(&v, &nrm, sizeof(v));
	}

	void GLMMath::Rotate(Core::Vector3Float& res, const Core::Vector3Float& v, const Core::Quaternion& r) const
	{
		const auto& glmV = reinterpret_cast<const glm::vec3&>(v);
		const auto& glmR = reinterpret_cast<const glm::quat&>(r);
		const auto glmRes = glmR * glmV;

		std::memcpy(&res, &glmRes, sizeof(res));
	}

	void GLMMath::Scale(Core::Matrix4x4& res, const Core::Vector3Float& scale) const
	{
		const auto& glmScale = reinterpret_cast<const glm::vec3&>(scale);
		const auto glmRes = glm::scale(glmScale);

		std::memcpy(&res, &glmRes, sizeof(res));
	}

	void GLMMath::Translation(Core::Matrix4x4& res, const Core::Vector3Float& translation) const
	{
		const auto& glmTranslation = reinterpret_cast<const glm::vec3&>(translation);
		const auto glmRes = glm::transpose(glm::translate(glmTranslation));

		std::memcpy(&res, &glmRes, sizeof(res));
	}

	void GLMMath::Rotation(Core::Matrix4x4& res, const Core::Quaternion& rotation) const
	{
		const auto& glmRot = reinterpret_cast<const glm::quat&>(rotation);
		const auto glmRes = glm::transpose(glm::toMat4(glmRot));

		std::memcpy(&res, &glmRes, sizeof(res));
	}

	void GLMMath::DecomposeTRS(Core::Transform& transform, const Core::Matrix4x4& matrix) const
	{
		auto& scale = reinterpret_cast<glm::vec3&>(transform.scale);
		auto& rotation = reinterpret_cast<glm::quat&>(transform.rotation);
		auto& translation = reinterpret_cast<glm::vec3&>(transform.position);
		glm::vec3 skew {};
		glm::vec4 perspective {};

		const auto glmMatrix = glm::transpose(reinterpret_cast<const glm::mat4&>(matrix));
		glm::decompose(glmMatrix, scale, rotation, translation, skew, perspective);
	}


#undef GLM_ENABLE_EXPERIMENTAL
}
