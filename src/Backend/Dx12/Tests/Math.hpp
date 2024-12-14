#pragma once
#include <Core/Tests/Math.hpp>

namespace MMPEngine::Backend::Dx12::Tests
{
	class DxMathProvider : public MMPEngine::Core::Tests::BaseMathProvider
	{
	public:
		std::unique_ptr<Core::Math> Make() override;
	};
}