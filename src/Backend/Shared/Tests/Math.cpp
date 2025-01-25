#include <Backend/Shared/Tests/Math.hpp>
#include <Backend/Shared/Math.hpp>

namespace MMPEngine::Backend::Shared::Tests
{
	std::unique_ptr<Core::Math> GLMMathProvider::Make()
	{
		return std::make_unique<Shared::GLMMath>();
	}
}

namespace MMPEngine::Core::Tests
{
	typedef testing::Types<MMPEngine::Backend::Shared::Tests::GLMMathProvider> SharedImplementations;
	INSTANTIATE_TYPED_TEST_SUITE_P(MathTests,
		MathTests,
		SharedImplementations
	);
}