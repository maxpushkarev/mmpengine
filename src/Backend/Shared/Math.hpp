#pragma once
#include <Core/Math.hpp>

namespace MMPEngine::Backend::Shared
{
	class GLMMath final : public Core::Math
	{
	public:
		std::float_t Dot(const Core::Vector3Float& v1, const Core::Vector3Float& v2) const override;

	};
}