#include <Backend/Metal/Tests/Math.hpp>
#include <Backend/Metal/Math.hpp>

namespace MMPEngine::Backend::Metal::Tests
{
	std::unique_ptr<Core::Math> MathProvider::Make()
	{
		return std::make_unique<Metal::Math>();
	}
}

namespace MMPEngine::Core::Tests
{
	typedef testing::Types<MMPEngine::Backend::Metal::Tests::MathProvider> MetalImplementations;
	INSTANTIATE_TYPED_TEST_SUITE_P(MathTests,
		MathTests,
		MetalImplementations
	);
}
