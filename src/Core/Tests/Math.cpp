#include <gtest/gtest.h>
#include <Core/Math.hpp>

namespace MMPEngine::Core::Tests
{
	class MathTests : public testing::Test
	{
	protected:
		std::unique_ptr<Core::DefaultMath> _default;

		inline void SetUp() override
		{
			testing::Test::SetUp();
			_default = std::make_unique<Core::DefaultMath>();
		}

		inline void TearDown() override
		{
			_default.reset();
			testing::Test::TearDown();
		}
	};


	TEST_F(MathTests, ConvertDegreesToRadians)
	{
		EXPECT_FLOAT_EQ(0.5f * Core::Math::kPi, Core::Math::ConvertDegreesToRadians(90.0f));
	}
}