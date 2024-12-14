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
		const Core::Vector3Float v {0.56f, -4.892f, 3.784f };
		Core::Vector3Float res1 = v;
		Core::Vector3Float res2 = v;

		this->_default->Normalize(res1);
		this->_mathImpl->Normalize(res2);

		EXPECT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Vector3_Magnitude)
	{
		const Core::Vector3Float v {0.56f, -4.892f, 3.784f };

		const auto res1 = this->_default->Magnitude(v);
		const auto res2 = this->_mathImpl->Magnitude(v);

		EXPECT_FLOAT_EQ(res1, res2);

	}

	TYPED_TEST_P(MathTests, Vector3_SquaredMagnitude)
	{
		const Core::Vector3Float v {0.56f, -4.892f, 3.784f };

		const auto res1 = this->_default->SquaredMagnitude(v);
		const auto res2 = this->_mathImpl->SquaredMagnitude(v);

		EXPECT_FLOAT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Matrix4x4_TRS)
	{
	}

	TYPED_TEST_P(MathTests, Matrix4x4_Multiply)
	{
	}

	TYPED_TEST_P(MathTests, Matrix4x4_Multiply_Vector4)
	{
	}

	TYPED_TEST_P(MathTests, Matrix4x4_Inverse)
	{
	}

	TYPED_TEST_P(MathTests, Matrix4x4_Transpose)
	{
	}

	REGISTER_TYPED_TEST_SUITE_P(
		MathTests,
		Vector3_Dot,
		Vector3_Cross,
		Vector3_Magnitude,
		Vector3_Normalize,
		Vector3_SquaredMagnitude,
		Matrix4x4_Inverse,
		Matrix4x4_Multiply,
		Matrix4x4_Multiply_Vector4,
		Matrix4x4_TRS,
		Matrix4x4_Transpose);
}