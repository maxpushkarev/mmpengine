#pragma once
#include <gtest/gtest.h>
#include <Core/Math.hpp>

namespace MMPEngine::Core::Tests
{
	class BaseMathProvider
	{
	public:
		BaseMathProvider() = default;
		BaseMathProvider(const BaseMathProvider&) = delete;
		BaseMathProvider(BaseMathProvider&&) noexcept = delete;
		BaseMathProvider& operator=(const BaseMathProvider&) = delete;
		BaseMathProvider& operator=(BaseMathProvider&&) noexcept = delete;
		virtual ~BaseMathProvider() = default;

		virtual std::unique_ptr<Core::Math> Make() = 0;
	};

	template<typename TMathProvider>
	class MathTests : public testing::Test
	{
		static_assert(std::is_base_of_v<BaseMathProvider, TMathProvider>);
	protected:

		std::unique_ptr<Core::Math> _mathImpl;
		std::unique_ptr<Core::DefaultMath> _default;

		inline void SetUp() override
		{
			testing::Test::SetUp();
			TMathProvider provider {};
			_mathImpl = provider.Make();
			_default = std::make_unique<Core::DefaultMath>();
		}

		inline void TearDown() override
		{
			_mathImpl.reset();
			_default.reset();
			testing::Test::TearDown();
		}
	};

	TYPED_TEST_SUITE_P(MathTests);

	TYPED_TEST_P(MathTests, Vector3_Dot)
	{
		Core::Vector3Float v1 {0.56f, -4.892f, 3.784f };
		Core::Vector3Float v2 { -10.84f,-3.29f,0.61f };

		EXPECT_EQ(this->_default->Dot(v1, v2), this->_mathImpl->Dot(v1, v2));
	}

	TYPED_TEST_P(MathTests, Vector3_Cross)
	{
		Core::Vector3Float v1 {0.56f, -4.892f, 3.784f };
		Core::Vector3Float v2 { -10.84f, -3.29f, 0.61f };

		Core::Vector3Float res1 {};
		Core::Vector3Float res2 {};

		this->_default->Cross(res1, v1, v2);
		this->_mathImpl->Cross(res2, v1, v2);

		EXPECT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Vector3_Normalize)
	{
		constexpr  Core::Vector3Float v {0.56f, -4.892f, 3.784f };
		Core::Vector3Float res1 = v;
		Core::Vector3Float res2 = v;

		this->_default->Normalize(res1);
		this->_mathImpl->Normalize(res2);

		EXPECT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Vector3_Project_On_Vector3)
	{
		Core::Vector3Float v1 {0.56f, -4.892f, 3.784f };
		Core::Vector3Float v2 { -10.84f, -3.29f, 0.61f };

		Core::Vector3Float res1 {};
		Core::Vector3Float res2 {};

		this->_default->Project(res1, v1, v2);
		this->_mathImpl->Project(res2, v1, v2);

		EXPECT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Vector3_Magnitude)
	{
		constexpr  Core::Vector3Float v {0.56f, -4.892f, 3.784f };

		const auto res1 = this->_default->Magnitude(v);
		const auto res2 = this->_mathImpl->Magnitude(v);

		EXPECT_FLOAT_EQ(res1, res2);

	}

	TYPED_TEST_P(MathTests, Vector3_SquaredMagnitude)
	{
		constexpr Core::Vector3Float v {0.56f, -4.892f, 3.784f };

		const auto res1 = this->_default->SquaredMagnitude(v);
		const auto res2 = this->_mathImpl->SquaredMagnitude(v);

		EXPECT_FLOAT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Matrix4x4_TRS)
	{
	}

	TYPED_TEST_P(MathTests, Matrix4x4_Multiply)
	{
		constexpr Core::Matrix4x4 m1 {
			{
				{-1.457f, 3.333f, 0.049f, -8.24f},
				{ -6.094f, 6.912f, -2.693f, 5.85f },
				{ 9.42f, -0.018f, 7.12f, -4.12f },
				{ -1.12f, 4.141f, 5.1177f, 1.0125f }
			}
		};

		constexpr Core::Matrix4x4 m2 {
			{
				{8.92f, 2.47f, -3.74f, 4.86f},
				{ 9.711f, -1.83f, 6.932f, -8.03f },
				{ -1.9282f, 1.241f, -2.17f, 2.59f },
				{ 3.586f, -4.62f, 0.672f, -4.747f }
			}
		};

		Core::Matrix4x4 res1 {};
		Core::Matrix4x4 res2 {};

		this->_default->Multiply(res1, m1, m2);
		this->_mathImpl->Multiply(res2, m1, m2);

		EXPECT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Matrix4x4_Multiply_Point)
	{
		constexpr Core::Matrix4x4 m {
			{
				{-1.457f, 3.333f, 0.049f, -8.24f},
				{ -6.094f, 6.912f, -2.693f, 5.85f },
				{ 9.42f, -0.018f, 7.12f, -4.12f },
				{ -1.12f, 4.141f, 5.1177f, 1.0125f }
			}
		};

		constexpr Core::Vector3Float p {0.56f, -4.892f, 3.784f };

		Core::Vector3Float res1 {};
		Core::Vector3Float res2 {};

		this->_default->MultiplyMatrixAndPoint(res1, m, p);
		this->_mathImpl->MultiplyMatrixAndPoint(res2, m, p);

		ASSERT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Matrix4x4_Multiply_Vector4)
	{
		constexpr Core::Matrix4x4 m {
			{
				{-1.457f, 3.333f, 0.049f, -8.24f},
				{ -6.094f, 6.912f, -2.693f, 5.85f },
				{ 9.42f, -0.018f, 7.12f, -4.12f },
				{ -1.12f, 4.141f, 5.1177f, 1.0125f }
			}
		};

		constexpr Core::Vector4Float v {0.56f, -4.892f, 3.784f, 1.0f };

		Core::Vector4Float res1 {};
		Core::Vector4Float res2 {};

		this->_default->Multiply(res1, m, v);
		this->_mathImpl->Multiply(res2, m, v);

		ASSERT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Matrix4x4_Multiply_Vector3)
	{
		constexpr Core::Matrix4x4 m {
			{
				{-1.457f, 3.333f, 0.049f, -8.24f},
				{ -6.094f, 6.912f, -2.693f, 5.85f },
				{ 9.42f, -0.018f, 7.12f, -4.12f },
				{ -1.12f, 4.141f, 5.1177f, 1.0125f }
			}
		};

		constexpr Core::Vector3Float v {0.56f, -4.892f, 3.784f };

		Core::Vector3Float res1 {};
		Core::Vector3Float res2 {};

		this->_default->MultiplyMatrixAndVector(res1, m, v);
		this->_mathImpl->MultiplyMatrixAndVector(res2, m, v);

		ASSERT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Matrix4x4_Inverse)
	{
		constexpr Core::Matrix4x4 m {
			{
				{-1.457f, 3.333f, 0.049f, -8.24f},
				{ -6.094f, 6.912f, -2.693f, 5.85f },
				{ 9.42f, -0.018f, 7.12f, -4.12f },
				{ -1.12f, 4.141f, 5.1177f, 1.0125f }
			}
		};

		Core::Matrix4x4 res1 {};
		this->_mathImpl->Inverse(res1, m);

		Core::Matrix4x4 identity1 {};
		this->_mathImpl->Multiply(identity1, m, res1);

		ASSERT_EQ(Core::Math::kMatrix4x4Identity, identity1);
	}

	TYPED_TEST_P(MathTests, Matrix4x4_Transpose)
	{
		constexpr Core::Matrix4x4 m {
			{
				{-1.457f, 3.333f, 0.049f, -8.24f},
				{ -6.094f, 6.912f, -2.693f, 5.85f },
				{ 9.42f, -0.018f, 7.12f, -4.12f },
				{ -1.12f, 4.141f, 5.1177f, 1.0125f }
			}
		};

		Core::Matrix4x4 res1 {};
		Core::Matrix4x4 res2 {};

		this->_default->Transpose(res1, m);
		this->_mathImpl->Transpose(res2, m);

		ASSERT_EQ(res1, res2);
	}

	REGISTER_TYPED_TEST_SUITE_P(
		MathTests,
		Vector3_Dot,
		Vector3_Cross,
		Vector3_Magnitude,
		Vector3_Normalize,
		Vector3_SquaredMagnitude,
		Vector3_Project_On_Vector3,
		Matrix4x4_Inverse,
		Matrix4x4_Multiply,
		Matrix4x4_Multiply_Point,
		Matrix4x4_Multiply_Vector3,
		Matrix4x4_Multiply_Vector4,
		Matrix4x4_TRS,
		Matrix4x4_Transpose);
}