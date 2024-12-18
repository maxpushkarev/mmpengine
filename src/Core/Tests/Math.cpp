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


	TEST_F(MathTests, GetRow)
	{
		constexpr Core::Matrix4x4 m {
			{
				{ 1.0f, 2.0f, 3.0f, 4.0f },
				{ 5.0f, 6.0f, 7.0f, 8.0f },
				{ 9.0f, 10.0f, 11.0f, 12.0f },
				{ 13.0f, 14.0f, 15.0f, 16.0f }
			}
		};

		Vector3Float v3 {};
		Vector4Float v4 {};

		Core::Math::GetRow(v3, 1, m);
		Core::Math::GetRow(v4, 1, m);

		ASSERT_EQ((Vector3Float{ 5.0f,6.0f,7.0f }), v3);
		ASSERT_EQ((Vector4Float{ 5.0f,6.0f,7.0f,8.0f }), v4);
	};

	TEST_F(MathTests, GetColumn)
	{
		constexpr Core::Matrix4x4 m {
			{
				{ 1.0f, 2.0f, 3.0f, 4.0f },
				{ 5.0f, 6.0f, 7.0f, 8.0f },
				{ 9.0f, 10.0f, 11.0f, 12.0f },
				{ 13.0f, 14.0f, 15.0f, 16.0f }
			}
		};

		Vector3Float v3{};
		Vector4Float v4{};

		Core::Math::GetColumn(v3, 1, m);
		Core::Math::GetColumn(v4, 1, m);

		ASSERT_EQ((Vector3Float{ 2.0f,6.0f,10.0f }), v3);
		ASSERT_EQ((Vector4Float{ 2.0f,6.0f,10.0f,14.0f }), v4);
	};
}