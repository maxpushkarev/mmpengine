#pragma once
#include <Core/Tests/Math.hpp>

namespace MMPEngine::Backend::Metal::Tests
{
	class MathProvider : public MMPEngine::Core::Tests::BaseMathProvider
	{
	public:
		std::unique_ptr<Core::Math> Make() override;
	};
}
