#include <Backend/Dx12/Tests/Math.hpp>
#include <Backend/Dx12/Math.hpp>

namespace MMPEngine::Backend::Dx12::Tests
{
	std::unique_ptr<Core::Math> MathProvider::Make()
	{
		return std::make_unique<Dx12::Math>();
	}
}

namespace MMPEngine::Core::Tests
{
	typedef testing::Types<MMPEngine::Backend::Dx12::Tests::MathProvider> Dx12Implementations;
	INSTANTIATE_TYPED_TEST_SUITE_P(MathTests,
		MathTests,
		Dx12Implementations
	);
}