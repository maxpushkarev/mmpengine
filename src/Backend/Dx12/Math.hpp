#pragma once
#include <Core/Math.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Math final : public Core::Math
	{
	public:
		std::float_t Dot(const Core::Vector3Float& v1, const Core::Vector3Float& v2) const override;
		void Cross(Core::Vector3Float& res, const Core::Vector3Float& v1, const Core::Vector3Float& v2) const override;
		std::float_t Magnitude(const Core::Vector3Float& v) const override;
		std::float_t SquaredMagnitude(const Core::Vector3Float& v) const override;
		void Normalize(Core::Vector3Float& v) const override;
	};
}