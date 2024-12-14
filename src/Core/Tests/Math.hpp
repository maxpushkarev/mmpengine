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

		std::unique_ptr<Core::Math> _math;

		inline void SetUp() override
		{
			testing::Test::SetUp();
			TMathProvider provider {};
			_math = provider.Make();
		}

		inline void TearDown() override
		{
			_math.reset();
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

	REGISTER_TYPED_TEST_SUITE_P(
		MathTests,
		Vector3_Dot,
		Vector3_Cross,
		Vector3_Magnitude,
		Vector3_Normalize,
		Vector3_SquaredMagnitude);
}