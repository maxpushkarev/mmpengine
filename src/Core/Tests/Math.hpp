#pragma once
#include <gtest/gtest.h>
#include <Core/Math.hpp>
#include <Core/Context.hpp>
#include <Core/Node.hpp>

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

	private:
		std::unique_ptr<Core::Math> _mathImpl;
		std::unique_ptr<Core::Math> _defaultMath;
	protected:
		inline const Core::Math* GetDefaultMath() const
		{
			return _defaultMath.get();
		}

		inline const Core::Math* GetMathImpl() const
		{
			return _mathImpl.get();
		}

		inline void SetUp() override
		{
			testing::Test::SetUp();
			TMathProvider provider {};

			_defaultMath = std::make_unique<Core::DefaultMath>();
			_mathImpl = provider.Make();

		}

		inline void TearDown() override
		{
			_defaultMath.reset();
			_mathImpl.reset();
			testing::Test::TearDown();
		}
	};

	TYPED_TEST_SUITE_P(MathTests);

	TYPED_TEST_P(MathTests, Vector3_Dot)
	{
		Core::Vector3Float v1 {0.56f, -4.892f, 3.784f };
		Core::Vector3Float v2 { -10.84f,-3.29f,0.61f };

        ASSERT_FLOAT_EQ(this->GetDefaultMath()->Dot(v1, v2), this->GetMathImpl()->Dot(v1, v2));
	}

	TYPED_TEST_P(MathTests, Vector3_Cross)
	{
		Core::Vector3Float v1 {0.56f, -4.892f, 3.784f };
		Core::Vector3Float v2 { -10.84f, -3.29f, 0.61f };

		Core::Vector3Float res1 {};
		Core::Vector3Float res2 {};

		this->GetDefaultMath()->Cross(res1, v1, v2);
		this->GetMathImpl()->Cross(res2, v1, v2);

		EXPECT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Vector3_Normalize)
	{
		constexpr  Core::Vector3Float v {0.56f, -4.892f, 3.784f };
		Core::Vector3Float res1 = v;
		Core::Vector3Float res2 = v;

		this->GetDefaultMath()->Normalize(res1);
		this->GetMathImpl()->Normalize(res2);

		EXPECT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Vector3_Project_On_Vector3)
	{
		Core::Vector3Float v1 {0.56f, -4.892f, 3.784f };
		Core::Vector3Float v2 { -10.84f, -3.29f, 0.61f };

		Core::Vector3Float res1 {};
		Core::Vector3Float res2 {};

		this->GetDefaultMath()->Project(res1, v1, v2);
		this->GetMathImpl()->Project(res2, v1, v2);

		EXPECT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Vector3_Magnitude)
	{
		constexpr  Core::Vector3Float v {0.56f, -4.892f, 3.784f };

		const auto res1 = this->GetDefaultMath()->Magnitude(v);
		const auto res2 = this->GetMathImpl()->Magnitude(v);

		EXPECT_FLOAT_EQ(res1, res2);

	}

	TYPED_TEST_P(MathTests, Vector3_SquaredMagnitude)
	{
		constexpr Core::Vector3Float v {0.56f, -4.892f, 3.784f };

		const auto res1 = this->GetDefaultMath()->SquaredMagnitude(v);
		const auto res2 = this->GetMathImpl()->SquaredMagnitude(v);

		EXPECT_FLOAT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Vector3_Rotate)
	{
		constexpr Core::Vector3Float v {0.56f, -4.892f, 3.784f };
		Core::Quaternion rot {};
		this->GetDefaultMath()->RotationAroundAxis(rot, { 1.0f, -5.39f, 2.43f }, Core::Math::ConvertDegreesToRadians(27.78f));

		Core::Vector3Float res1 {};
		Core::Vector3Float res2 {};

		this->GetDefaultMath()->Rotate(res1, v, rot);
		this->GetMathImpl()->Rotate(res2, v, rot);

		EXPECT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Matrix4x4_TRS)
	{
		Core::Transform t {
			{-5.25f, 14.4f, -3.71f},
			Core::Math::kQuaternionIdentity,
			{1.5f, 4.21f, 2.069f}
		};
		this->GetDefaultMath()->RotationAroundAxis(t.rotation, {1.0f, -5.39f, 2.43f}, Core::Math::ConvertDegreesToRadians(27.78f));

		Core::Matrix4x4 res1 {};
		Core::Matrix4x4 res2 {};

		this->GetDefaultMath()->TRS(res1, t);
		this->GetMathImpl()->TRS(res2, t);

		ASSERT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Matrix4x4_DecomposeTRS)
	{
		Core::Transform t {
			{-5.25f, 14.4f, -3.71f},
				Core::Math::kQuaternionIdentity,
			{ 1.5f, 4.21f, 2.069f }
		};
		this->GetDefaultMath()->RotationAroundAxis(t.rotation, { 1.0f, -5.39f, -2.43f }, Core::Math::ConvertDegreesToRadians(-127.78f));

		Core::Matrix4x4 m1 {};
		Core::Matrix4x4 m2 {};

		this->GetDefaultMath()->TRS(m1, t);
		this->GetMathImpl()->TRS(m2, t);

		Core::Transform res1 {};
		Core::Transform res2 {};

		this->GetDefaultMath()->DecomposeTRS(res1, m2);
		ASSERT_EQ(t.position, res1.position);
		ASSERT_EQ(t.scale, res1.scale);
		ASSERT_TRUE(Quaternion::AreIdentical(t.rotation, res1.rotation));

		this->GetMathImpl()->DecomposeTRS(res2, m2);
		ASSERT_EQ(t.position, res2.position);
		ASSERT_EQ(t.scale, res2.scale);
		ASSERT_TRUE(Quaternion::AreIdentical(t.rotation, res2.rotation));
	}

	TYPED_TEST_P(MathTests, Matrix4x4_Scale)
	{
		constexpr Core::Vector3Float scale = { 1.5f, 1.21f, 1.069f };
		Core::Matrix4x4 res1 {};
		Core::Matrix4x4 res2 {};

		this->GetDefaultMath()->Scale(res1, scale);
		this->GetMathImpl()->Scale(res2, scale);

		ASSERT_EQ(res1, res2);
	}


	TYPED_TEST_P(MathTests, Matrix4x4_Translation)
	{
		constexpr Core::Vector3Float translation = { -5.25f, 11.4f, 3.71f };
		Core::Matrix4x4 res1 {};
		Core::Matrix4x4 res2 {};

		this->GetDefaultMath()->Translation(res1, translation);
		this->GetMathImpl()->Translation(res2, translation);

		ASSERT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Matrix4x4_Rotation)
	{
		Core::Quaternion rotation {};
		this->GetDefaultMath()->RotationAroundAxis(rotation, { 1.0f, 1.0f, 0.0f }, Core::Math::ConvertDegreesToRadians(30.0f));

		Core::Matrix4x4 res1 {};
		Core::Matrix4x4 res2 {};

		this->GetDefaultMath()->Rotation(res1, rotation);
		this->GetMathImpl()->Rotation(res2, rotation);

		ASSERT_EQ(res1, res2);
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

		this->GetDefaultMath()->Multiply(res1, m1, m2);
		this->GetMathImpl()->Multiply(res2, m1, m2);

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

		this->GetDefaultMath()->MultiplyMatrixAndPoint(res1, m, p);
		this->GetMathImpl()->MultiplyMatrixAndPoint(res2, m, p);

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

		this->GetDefaultMath()->Multiply(res1, m, v);
		this->GetMathImpl()->Multiply(res2, m, v);

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

		this->GetDefaultMath()->MultiplyMatrixAndVector(res1, m, v);
		this->GetMathImpl()->MultiplyMatrixAndVector(res2, m, v);

		ASSERT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Matrix4x4_Determinant)
	{
		constexpr Core::Matrix4x4 m {
			{
				{-1.457f, 3.333f, 0.049f, -8.24f},
				{ -6.094f, 6.912f, -2.693f, 5.85f },
				{ 9.42f, -0.018f, 7.12f, -4.12f },
				{ -1.12f, 4.141f, 5.1177f, 1.0125f }
			}
		};

		ASSERT_EQ(this->GetDefaultMath()->Determinant(m), this->GetMathImpl()->Determinant(m));
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
		this->GetMathImpl()->Inverse(res1, m);

		Core::Matrix4x4 identity1 {};
		this->GetMathImpl()->Multiply(identity1, m, res1);

		ASSERT_EQ(Core::Math::kMatrix4x4Identity, identity1);

		Core::Matrix4x4 res2 {};
		this->GetDefaultMath()->Inverse(res2, m);

		Core::Matrix4x4 identity2 {};
		this->GetDefaultMath()->Multiply(identity2, m, res2);

		ASSERT_EQ(Core::Math::kMatrix4x4Identity, identity2);
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

		this->GetDefaultMath()->Transpose(res1, m);
		this->GetMathImpl()->Transpose(res2, m);

		ASSERT_EQ(res1, res2);
	}

	TYPED_TEST_P(MathTests, Matrix4x4_InverseTranspose)
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

		this->GetDefaultMath()->InverseTranspose(res1, m);
		this->GetMathImpl()->InverseTranspose(res2, m);

		ASSERT_EQ(res1, res2);
	}


	TYPED_TEST_P(MathTests, Quaternion_RotateAroundAxis)
	{
		constexpr Core::Vector3Float v {1.0f, 1.0f, -1.0f};
		const auto rad = Core::Math::ConvertDegreesToRadians(30);

		Core::Quaternion res1 {};
		Core::Quaternion res2 {};

		this->GetDefaultMath()->RotationAroundAxis(res1, v, rad);
		this->GetMathImpl()->RotationAroundAxis(res2, v, rad);

		ASSERT_TRUE(Quaternion::AreIdentical(res1, res2));
	}

	TYPED_TEST_P(MathTests, Quaternion_FromEuler)
	{
		for (std::int32_t a = -360; a <= 360; a += 5)
		{
			const auto singleAngle = Core::Math::ConvertDegreesToRadians(static_cast<std::float_t>(a));

			Core::Quaternion expectedX;
			Core::Quaternion expectedY;
			Core::Quaternion expectedZ;

			this->GetDefaultMath()->RotationAroundAxis(expectedX, Core::Vector3Float {1.0f, 0.0f, 0.0f }, singleAngle);
			this->GetDefaultMath()->RotationAroundAxis(expectedY, Core::Vector3Float{ 0.0f, 1.0f, 0.0f }, singleAngle);
			this->GetDefaultMath()->RotationAroundAxis(expectedZ, Core::Vector3Float{ 0.0f, 0.0f, 1.0f }, singleAngle);

			Core::Quaternion actualX;
			Core::Quaternion actualY;
			Core::Quaternion actualZ;

			this->GetMathImpl()->RotationFromEuler(actualX, Core::Vector3Float{ singleAngle, 0.0f, 0.0f });
			this->GetMathImpl()->RotationFromEuler(actualY, Core::Vector3Float{ 0.0f, singleAngle, 0.0f });
			this->GetMathImpl()->RotationFromEuler(actualZ, Core::Vector3Float{ 0.0f, 0.0f, singleAngle });

			ASSERT_TRUE(Core::Quaternion::AreEquivalent(expectedX, actualX));
			ASSERT_TRUE(Core::Quaternion::AreEquivalent(expectedY, actualY));
			ASSERT_TRUE(Core::Quaternion::AreEquivalent(expectedZ, actualZ));

			this->GetDefaultMath()->RotationFromEuler(actualX, Core::Vector3Float{ singleAngle, 0.0f, 0.0f });
			this->GetDefaultMath()->RotationFromEuler(actualY, Core::Vector3Float{ 0.0f, singleAngle, 0.0f });
			this->GetDefaultMath()->RotationFromEuler(actualZ, Core::Vector3Float{ 0.0f, 0.0f, singleAngle });

			ASSERT_TRUE(Core::Quaternion::AreEquivalent(expectedX, actualX));
			ASSERT_TRUE(Core::Quaternion::AreEquivalent(expectedY, actualY));
			ASSERT_TRUE(Core::Quaternion::AreEquivalent(expectedZ, actualZ));
		}

		for(std::int32_t x = -360; x <= 360; x+=5)
		{
			for (std::int32_t y = -360; y <= 360; y += 5)
			{
				for (std::int32_t z = -360; z <= 360; z += 5)
				{
					const Core::Vector3Float customEulerAngles {
						Core::Math::ConvertDegreesToRadians(static_cast<std::float_t>(x)),
							Core::Math::ConvertDegreesToRadians(static_cast<std::float_t>(y)),
							Core::Math::ConvertDegreesToRadians(static_cast<std::float_t>(z)),
					};

					Core::Quaternion q1 {};
					Core::Quaternion q2 {};

					this->GetDefaultMath()->RotationFromEuler(q1, customEulerAngles);
					this->GetMathImpl()->RotationFromEuler(q2, customEulerAngles);

					ASSERT_TRUE(Core::Quaternion::AreEquivalent(q1, q2));
				}
			}
		}
	}

	TYPED_TEST_P(MathTests, Quaternion_Dot)
	{
		constexpr Core::Quaternion q1 {0.1f, 0.2f, 0.3f, 0.4f};
		constexpr Core::Quaternion q2 {0.5f, 0.6f, 0.7f, 0.8f};

		ASSERT_FLOAT_EQ(this->GetDefaultMath()->Dot(q1, q2), this->GetMathImpl()->Dot(q1, q2));
	}

	TYPED_TEST_P(MathTests, Quaternion_Multiply)
	{
		constexpr Core::Quaternion q1 {0.1f, 0.2f, 0.3f, 0.4f};
		constexpr Core::Quaternion q2 {0.5f, 0.6f, 0.7f, 0.8f};

		Core::Quaternion res1 {};
		Core::Quaternion res2 {};

		this->GetDefaultMath()->Multiply(res1, q1, q2);
		this->GetMathImpl()->Multiply(res2, q1, q2);

		ASSERT_TRUE(Quaternion::AreIdentical(res1, res2));
	}

	TYPED_TEST_P(MathTests, Quaternion_Inverse)
	{
		constexpr Core::Quaternion q {0.1f, 0.2f, 0.3f, 0.4f};

		Core::Quaternion res1 {};
		Core::Quaternion res2 {};

		this->GetDefaultMath()->Inverse(res1, q);
		this->GetMathImpl()->Inverse(res2, q);

		Core::Quaternion identity1 {};
		Core::Quaternion identity2 {};

		this->GetDefaultMath()->Multiply(identity1, q, res1);
		this->GetMathImpl()->Multiply(identity2, q, res2);

		ASSERT_TRUE(Quaternion::AreIdentical(Core::Math::kQuaternionIdentity, identity1));
		ASSERT_TRUE(Quaternion::AreIdentical(Core::Math::kQuaternionIdentity, identity2));
	}


	TYPED_TEST_P(MathTests, Node_LocalToWorld)
	{
		Core::Transform t1 {
			{-5.25f, 14.4f, -3.71f},
				Core::Math::kQuaternionIdentity,
			{ 1.5f, 4.21f, 2.069f }
		};
		this->GetDefaultMath()->RotationAroundAxis(t1.rotation, { 1.0f, -5.39f, 2.43f }, Core::Math::ConvertDegreesToRadians(-27.78f));

		Core::Transform t2 {
			{0.25f, 1.44f, -37.18f},
				Core::Math::kQuaternionIdentity,
			{ 8.5f, 1.21f, 0.34f }
		};
		this->GetDefaultMath()->RotationAroundAxis(t2.rotation, { -3.4f, -2.2f, 0.1f }, Core::Math::ConvertDegreesToRadians(35.5f));

		Core::Transform t3 {
			{3.81f, -9.2f, 4.69f},
				Core::Math::kQuaternionIdentity,
			{ 0.23f, 1.85f, 3.98f }
		};
		this->GetDefaultMath()->RotationAroundAxis(t3.rotation, { -3.4f, -2.2f, 0.1f }, Core::Math::ConvertDegreesToRadians(100.34f));

		const auto node1 = std::make_shared<Node>();
		const auto node2 = std::make_shared<Node>();
		const auto node3 = std::make_shared<Node>();


		node1->AddChild(node2);
		node2->AddChild(node3);

		node1->localTransform = t1;
		node2->localTransform = t2;
		node3->localTransform = t3;

		const auto checkNode = [this](const std::shared_ptr<Node>& node)
		{
			Core::Matrix4x4 res1 {};
			Core::Matrix4x4 res2 {};

			this->GetDefaultMath()->CalculateLocalToWorldSpaceMatrix(res1, node);
			this->GetMathImpl()->CalculateLocalToWorldSpaceMatrix(res2, node);
			ASSERT_EQ(res1, res2);
		};

		checkNode(node1);
		checkNode(node2);
		checkNode(node3);
	}

	TYPED_TEST_P(MathTests, Node_WorldTransform)
	{
		Core::Transform t1 {
			{-5.25f, 14.4f, -3.71f},
				Core::Math::kQuaternionIdentity,
			{ 1.12f, 2.6f, 4.91f }
		};
		this->GetDefaultMath()->RotationAroundAxis(t1.rotation, { 1.0f, -2.6f, 11.2f }, Core::Math::ConvertDegreesToRadians(30.0f));

		Core::Transform t2 {
			{0.25f, 1.44f, -37.18f},
				Core::Math::kQuaternionIdentity,
			{ 8.5f, 1.21f, 3.34f }
		};
		this->GetDefaultMath()->RotationAroundAxis(t2.rotation, { -4.13f, 1.0f, 0.0f }, Core::Math::ConvertDegreesToRadians(45.0f));

		Core::Transform t3 {
			{3.81f, -9.2f, 4.69f},
				Core::Math::kQuaternionIdentity,
			{ 0.23f, 1.85f, 7.98f }
		};
		this->GetDefaultMath()->RotationAroundAxis(t3.rotation, { -5.0f, 2.78f, -1.0f }, Core::Math::ConvertDegreesToRadians(60.0f));

		const auto node1 = std::make_shared<Node>();
		const auto node2 = std::make_shared<Node>();
		const auto node3 = std::make_shared<Node>();

		node1->AddChild(node2);
		node2->AddChild(node3);

		node1->localTransform = t1;
		node2->localTransform = t2;
		node3->localTransform = t3;

		const auto testNode = [this](const std::shared_ptr<Node>& node, bool checkScalingAsIdentity)
		{
			const auto kernel = [&node](const Core::Math* math, bool checkScalingAsIdentity)
			{
				Core::Transform t {};
				Core::Matrix4x4 s {};
				math->CalculateWorldSpaceTransform(t.position, t.rotation, s, node);

				if(checkScalingAsIdentity)
				{
					ASSERT_EQ(Core::Math::kMatrix4x4Identity, s);
				}

				Core::Matrix4x4 localToWorld {};
				math->CalculateLocalToWorldSpaceMatrix(localToWorld, node);

				Core::Matrix4x4 translation {};
				math->Translation(translation, t.position);

				Core::Matrix4x4 rotation {};
				math->Rotation(rotation, t.rotation);

				Core::Matrix4x4 tmp {};
				Core::Matrix4x4 actualTrs {};

				math->Multiply(tmp, translation, rotation);
				math->Multiply(actualTrs, tmp, s);

				ASSERT_EQ(localToWorld, actualTrs);
			};

			kernel(this->GetDefaultMath(), checkScalingAsIdentity);
			kernel(this->GetMathImpl(), checkScalingAsIdentity);
		};

		testNode(node1, false);
		testNode(node2, false);
		testNode(node3, false);

		node1->localTransform.scale = {1.0f, 1.0f, 1.0f };
		node2->localTransform.scale = {1.0f, 1.0f, 1.0f };
		node3->localTransform.scale = {1.0f, 1.0f, 1.0f };

		testNode(node1, true);
		testNode(node2, true);
		testNode(node3, true);
	}


	REGISTER_TYPED_TEST_SUITE_P(
		MathTests,
		Vector3_Dot,
		Vector3_Cross,
		Vector3_Magnitude,
		Vector3_Normalize,
		Vector3_Rotate,
		Vector3_SquaredMagnitude,
		Vector3_Project_On_Vector3,
		Matrix4x4_Determinant,
		Matrix4x4_Inverse,
		Matrix4x4_Multiply,
		Matrix4x4_Multiply_Point,
		Matrix4x4_Multiply_Vector3,
		Matrix4x4_Multiply_Vector4,
		Matrix4x4_Scale,
		Matrix4x4_Translation,
		Matrix4x4_Rotation,
		Matrix4x4_TRS,
		Matrix4x4_DecomposeTRS,
		Matrix4x4_Transpose,
		Matrix4x4_InverseTranspose,
		Quaternion_RotateAroundAxis,
		Quaternion_Dot,
		Quaternion_Multiply,
		Quaternion_Inverse,
		Quaternion_FromEuler,
		Node_LocalToWorld,
		Node_WorldTransform);
}
