#include <Backend/Shared/Math.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "Core/Node.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/rotate_normalized_axis.hpp"

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

	void GLMMath::Multiply(Core::Matrix4x4& res, const Core::Matrix4x4& m1, const Core::Matrix4x4& m2) const
	{
		const auto& glmM1 = reinterpret_cast<const glm::mat4&>(m1);
		const auto& glmM2 = reinterpret_cast<const glm::mat4&>(m2);

		const auto glmRes = glmM2 * glmM1;
		std::memcpy(&res, &glmRes, sizeof(res));
	}

	void GLMMath::Multiply(Core::Vector4Float& res, const Core::Matrix4x4& m, const Core::Vector4Float& v) const
	{
		const auto& glmM = reinterpret_cast<const glm::mat4&>(m);
		const auto& glmV = reinterpret_cast<const glm::vec4&>(v);

		const auto glmRes = glmV * glmM;
		std::memcpy(&res, &glmRes, sizeof(res));
	}

	std::float_t GLMMath::Determinant(const Core::Matrix4x4& m) const
	{
		const auto& glmM = reinterpret_cast<const glm::mat4&>(m);
		return glm::determinant(glmM);
	}

	void GLMMath::Inverse(Core::Matrix4x4& res, const Core::Matrix4x4& m) const
	{
		const auto& glmM = glm::transpose(reinterpret_cast<const glm::mat4&>(m));

		const auto glmRes = glm::transpose(glm::inverse(glmM));
		std::memcpy(&res, &glmRes, sizeof(res));
	}

	void GLMMath::Transpose(Core::Matrix4x4& res, const Core::Matrix4x4& m) const
	{
		const auto& glmM = reinterpret_cast<const glm::mat4&>(m);

		const auto glmRes = glm::transpose(glmM);
		std::memcpy(&res, &glmRes, sizeof(res));
	}

	void GLMMath::InverseTranspose(Core::Matrix4x4& res, const Core::Matrix4x4& m) const
	{
		const auto& glmM = glm::transpose(reinterpret_cast<const glm::mat4&>(m));
		const auto glmRes = glm::inverse(glmM);
		std::memcpy(&res, &glmRes, sizeof(res));
	}

	void GLMMath::MultiplyMatrixAndPoint(Core::Vector3Float& res, const Core::Matrix4x4& m, const Core::Vector3Float& p) const
	{
		const glm::vec4 glmP = {p.x, p.y, p.z, 1.0f};
		const auto& glmM = reinterpret_cast<const glm::mat4&>(m);

		auto glmRes = glmP * glmM;
		const auto revW = 1.0f / glmRes.w;
		glmRes *= revW;

		std::memcpy(&res, &glmRes, sizeof(res));
	}

	void GLMMath::MultiplyMatrixAndVector(Core::Vector3Float& res, const Core::Matrix4x4& m, const Core::Vector3Float& v) const
	{
		const glm::vec4 glmV = { v.x, v.y, v.z, 0.0f };
		const auto& glmM = reinterpret_cast<const glm::mat4&>(m);

		const auto glmRes = glmV * glmM;
		std::memcpy(&res, &glmRes, sizeof(res));
	}

	std::float_t GLMMath::Dot(const Core::Quaternion& q1, const Core::Quaternion& q2) const
	{
		const auto& glmQ1 = reinterpret_cast<const glm::quat&>(q1);
		const auto& glmQ2 = reinterpret_cast<const glm::quat&>(q2);

		return glm::dot(glmQ1,glmQ2);
	}

	void GLMMath::Normalize(Core::Quaternion& q) const
	{
		const auto& glmQ = reinterpret_cast<const glm::quat&>(q);

		const auto glmRes = glm::normalize(glmQ);
		std::memcpy(&q, &glmRes, sizeof(q));
	}

	void GLMMath::Inverse(Core::Quaternion& res, const Core::Quaternion& q) const
	{
		const auto& glmQ = reinterpret_cast<const glm::quat&>(q);

		const auto glmRes = glm::inverse(glmQ);
		std::memcpy(&res, &glmRes, sizeof(res));
	}

	void GLMMath::Multiply(Core::Quaternion& res, const Core::Quaternion& q1, const Core::Quaternion& q2) const
	{
		const auto& glmQ1 = reinterpret_cast<const glm::quat&>(q1);
		const auto& glmQ2 = reinterpret_cast<const glm::quat&>(q2);

		const auto glmRes = glmQ1 * glmQ2;
		std::memcpy(&res, &glmRes, sizeof(res));
	}

	void GLMMath::RotationAroundAxis(Core::Quaternion& res, const Core::Vector3Float& v, std::float_t rad) const
	{
		const auto& glmV = reinterpret_cast<const glm::vec3&>(v);

		const auto glmRes = glm::rotate(glm::quat_identity<std::float_t, glm::defaultp>(), rad, glmV);
		std::memcpy(&res, &glmRes, sizeof(res));
	}

	void GLMMath::RotationFromEuler(Core::Quaternion& res, const Core::Vector3Float& eulerAngles) const
	{
		const glm::quat pitchQuat = glm::angleAxis(eulerAngles.x, glm::vec3(1.0f, 0.0f, 0.0f));
		const glm::quat yawQuat = glm::angleAxis(eulerAngles.y, glm::vec3(0.0f, 1.0f, 0.0f));
		const glm::quat rollQuat = glm::angleAxis(eulerAngles.z, glm::vec3(0.0f, 0.0f, 1.0f));

		const auto glmRes = yawQuat * pitchQuat * rollQuat;
		std::memcpy(&res, &glmRes, sizeof(res));
	}

	void GLMMath::CalculateLocalToWorldSpaceMatrix(Core::Matrix4x4& res, const std::shared_ptr<const Core::Node>& node) const
	{
		auto glmRes = glm::identity<glm::mat4::type>();
		auto currentNode = node;

		while (currentNode)
		{
			const auto glmTranslation = glm::translate(reinterpret_cast<const glm::vec3&>(currentNode->localTransform.position));
			const auto glmRotation = glm::toMat4(reinterpret_cast<const glm::quat&>(currentNode->localTransform.rotation));
			const auto glmScale = glm::scale(reinterpret_cast<const glm::vec3&>(currentNode->localTransform.scale));

			glmRes = glmTranslation * glmRotation * glmScale * glmRes;
			currentNode = currentNode->GetParent();
		}

		glmRes = glm::transpose(glmRes);
		std::memcpy(&res, &glmRes, sizeof(res));
	}


#undef GLM_ENABLE_EXPERIMENTAL
}
