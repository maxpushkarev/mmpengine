#pragma once
#include <Core/Math.hpp>

namespace MMPEngine::Backend::Metal
{
    class Math final : public Core::Math
    {
    public:
        std::float_t Dot(const Core::Vector3Float& v1, const Core::Vector3Float& v2) const override;
        void Cross(Core::Vector3Float& res, const Core::Vector3Float& v1, const Core::Vector3Float& v2) const override;
        std::float_t Magnitude(const Core::Vector3Float& v) const override;
        std::float_t SquaredMagnitude(const Core::Vector3Float& v) const override;
        void Normalize(Core::Vector3Float& v) const override;
        void Rotate(Core::Vector3Float& res, const Core::Vector3Float& v, const Core::Quaternion& r) const override;

        /*void Scale(Core::Matrix4x4& res, const Core::Vector3Float& scale) const override;
        void Translation(Core::Matrix4x4& res, const Core::Vector3Float& translation) const override;
        void Rotation(Core::Matrix4x4& res, const Core::Quaternion& rotation) const override;
        void TRS(Core::Matrix4x4& matrix, const Core::Transform& transform) const override;
        void DecomposeTRS(Core::Transform& transform, const Core::Matrix4x4& matrix) const override;
        void Multiply(Core::Matrix4x4& res, const Core::Matrix4x4& m1, const Core::Matrix4x4& m2) const override;
        void Multiply(Core::Vector4Float& res, const Core::Matrix4x4& m, const Core::Vector4Float& v) const override;
        std::float_t Determinant(const Core::Matrix4x4& m) const override;
        void Inverse(Core::Matrix4x4& res, const Core::Matrix4x4& m) const override;
        void Transpose(Core::Matrix4x4& res, const Core::Matrix4x4& m) const override;
        void InverseTranspose(Core::Matrix4x4& res, const Core::Matrix4x4& m) const override;
        void MultiplyMatrixAndPoint(Core::Vector3Float& res, const Core::Matrix4x4& m, const Core::Vector3Float& p) const override;
        void MultiplyMatrixAndVector(Core::Vector3Float& res, const Core::Matrix4x4& m, const Core::Vector3Float& v) const override;

        void RotationAroundAxis(Core::Quaternion& res, const Core::Vector3Float& v, std::float_t rad) const override;
        std::float_t Dot(const Core::Quaternion& q1, const Core::Quaternion& q2) const override;
        void Normalize(Core::Quaternion& q) const override;
        void Inverse(Core::Quaternion& res, const Core::Quaternion& q) const override;
        void RotationFromEuler(Core::Quaternion& res, const Core::Vector3Float& eulerAngles) const override;
        void Multiply(Core::Quaternion& res, const Core::Quaternion& q1, const Core::Quaternion& q2) const override;

        void CalculateLocalToWorldSpaceMatrix(Core::Matrix4x4& res, const std::shared_ptr<const Core::Node>& node) const override;
    private:
        static DirectX::XMMATRIX XM_CALLCONV TRSInternalTransposed(const Core::Transform& transform);*/
    };
}
