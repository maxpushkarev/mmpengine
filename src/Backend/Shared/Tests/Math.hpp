#pragma once
#include <Core/Tests/Math.hpp>

namespace MMPEngine::Backend::Shared::Tests
{
	class GLMMathProvider : public MMPEngine::Core::Tests::BaseMathProvider
	{
	public:
		std::unique_ptr<Core::Math> Make() override;
	};
}