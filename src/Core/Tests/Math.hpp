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
	}

	TYPED_TEST_P(MathTests, Vector3_Cross)
	{
	}

	TYPED_TEST_P(MathTests, Vector3_Normalize)
	{
	}

	TYPED_TEST_P(MathTests, Vector3_Magnitude)
	{
	}

	TYPED_TEST_P(MathTests, Vector3_SquaredMagnitude)
	{
	}

	TYPED_TEST_P(MathTests, Matrix4x4_TRS)
	{
	}

	TYPED_TEST_P(MathTests, Matrix4x4_Multiply)
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
		Matrix4x4_TRS,
		Matrix4x4_Transpose);
}