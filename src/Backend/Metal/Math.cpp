#include <Backend/Metal/Math.hpp>
#include <Core/Node.hpp>
#include <simd/simd.h>
#include <cassert>

namespace MMPEngine::Backend::Metal
{
    std::float_t Math::Dot(const Core::Vector3Float& v1, const Core::Vector3Float& v2) const
    {
        const auto& simdV1 = reinterpret_cast<const simd_float3&>(v1);
        const auto& simdV2 = reinterpret_cast<const simd_float3&>(v2);
       
        return simd_dot(simdV1, simdV2);
    }

    void Math::Cross(Core::Vector3Float& res, const Core::Vector3Float& v1, const Core::Vector3Float& v2) const
    {
        const auto& simdV1 = reinterpret_cast<const simd_float3&>(v1);
        const auto& simdV2 = reinterpret_cast<const simd_float3&>(v2);
        const auto cross = simd_cross(simdV1, simdV2);
        
        std::memcpy(&res, &cross, sizeof(res));
    }

    std::float_t Math::Magnitude(const Core::Vector3Float& v) const
    {
        const auto& simdV = reinterpret_cast<const simd_float3&>(v);
        return simd_length(simdV);
    }

    void Math::Normalize(Core::Vector3Float& v) const
    {
        const auto& simdV = reinterpret_cast<const simd_float3&>(v);
        const auto simdNrmV = simd_normalize(simdV);
        
        std::memcpy(&v, &simdNrmV, sizeof(v));
    }

    std::float_t Math::SquaredMagnitude(const Core::Vector3Float& v) const
    {
        const auto& simdV = reinterpret_cast<const simd_float3&>(v);
        return simd_length_squared(simdV);
    }

    void Math::Rotate(Core::Vector3Float& res, const Core::Vector3Float& v, const Core::Quaternion& r) const
    {
        const auto& simdV = reinterpret_cast<const simd_float3&>(v);
        const auto& simdR = reinterpret_cast<const simd_quatf&>(r);
        const auto rotated = simd_act(simdR, simdV);
        
        std::memcpy(&res, &rotated, sizeof(res));
    }

    void Math::Rotation(Core::Matrix4x4& res, const Core::Quaternion& rotation) const
    {
        const auto& simdR = reinterpret_cast<const simd_quatf&>(rotation);
        const auto mat = simd_transpose(simd_matrix4x4(simdR));
        std::memcpy(&res, &mat, sizeof(res));
    }


    void Math::Multiply(Core::Matrix4x4& res, const Core::Matrix4x4& m1, const Core::Matrix4x4& m2) const
    {
        const auto& simdM1 = reinterpret_cast<const simd_float4x4&>(m1.m);
        const auto& simdM2 = reinterpret_cast<const simd_float4x4&>(m2.m);
        
        const auto r = simd_mul(simdM2, simdM1);
        std::memcpy(&res, &r, sizeof(res));
    }

    void Math::Multiply(Core::Vector4Float& res, const Core::Matrix4x4& m, const Core::Vector4Float& v) const
    {
        const auto& simdV = reinterpret_cast<const simd_float4&>(v);
        const auto& simdM = reinterpret_cast<const simd_float4x4&>(m.m);
        const auto r = simd_mul(simdV, simdM);
        std::memcpy(&res, &r, sizeof(res));
    }

    std::float_t Math::Determinant(const Core::Matrix4x4& m) const
    {
        const auto& simdM = reinterpret_cast<const simd_float4x4&>(m.m);
        return simd_determinant(simd_transpose(simdM));
    }

    void Math::Inverse(Core::Matrix4x4& res, const Core::Matrix4x4& m) const
    {
        const auto& simdM = reinterpret_cast<const simd_float4x4&>(m.m);
        const auto inv = simd_inverse(simdM);
        std::memcpy(&res, &inv, sizeof(res));
    }

    void Math::Transpose(Core::Matrix4x4& res, const Core::Matrix4x4& m) const
    {
        const auto& simdM = reinterpret_cast<const simd_float4x4&>(m.m);
        const auto tr = simd_transpose(simdM);
        std::memcpy(&res, &tr, sizeof(res));
    }

    void Math::InverseTranspose(Core::Matrix4x4& res, const Core::Matrix4x4& m) const
    {
        const auto& simdM = reinterpret_cast<const simd_float4x4&>(m.m);
        const auto inv = simd_transpose(simd_inverse(simdM));
        std::memcpy(&res, &inv, sizeof(res));
    }

    void Math::MultiplyMatrixAndPoint(Core::Vector3Float& res, const Core::Matrix4x4& m, const Core::Vector3Float& p) const
    {
        const auto& simdP = reinterpret_cast<const simd_float3&>(p);
        const auto& simdM = reinterpret_cast<const simd_float4x4&>(m.m);
        const auto simdP4 = simd_make_float4(simdP, 1.0f);
        
        auto r = simd_mul(simdP4, simdM);
        const auto w = 1.0f / r.w;
        r.x *= w;
        r.y *= w;
        r.z *= w;
        
        std::memcpy(&res, &r, sizeof(res));
    }

    void Math::MultiplyMatrixAndVector(Core::Vector3Float& res, const Core::Matrix4x4& m, const Core::Vector3Float& v) const
    {
        const auto& simdV = reinterpret_cast<const simd_float3&>(v);
        const auto& simdM = reinterpret_cast<const simd_float4x4&>(m.m);
        const auto simdV4 = simd_make_float4(simdV, 0.0f);
        
        auto r = simd_mul(simdV4, simdM);
        std::memcpy(&res, &r, sizeof(res));
    }

    void Math::RotationAroundAxis(Core::Quaternion& res, const Core::Vector3Float& v, std::float_t rad) const
    {
        const auto& simdV = reinterpret_cast<const simd_float3&>(v);
        const auto nrm = simd_normalize(simdV);
        const auto q = simd_quaternion(rad, nrm);
        std::memcpy(&res, &q, sizeof(res));
    }

    std::float_t Math::Dot(const Core::Quaternion& q1, const Core::Quaternion& q2) const
    {
        const auto& simdQ1 = reinterpret_cast<const simd_quatf&>(q1);
        const auto& simdQ2 = reinterpret_cast<const simd_quatf&>(q2);

        return simd_dot(simdQ1, simdQ2);
    }

    void Math::Normalize(Core::Quaternion& q) const
    {
        const auto& simdQ = reinterpret_cast<const simd_quatf&>(q);
        const auto norm = simd_normalize(simdQ);
        std::memcpy(&q, &norm, sizeof(q));
    }

    void Math::Inverse(Core::Quaternion& res, const Core::Quaternion& q) const
    {
        const auto& simdQ = reinterpret_cast<const simd_quatf&>(q);
        const auto inv = simd_inverse(simdQ);
        std::memcpy(&res, &inv, sizeof(res));
    }

    void Math::RotationFromEuler(Core::Quaternion& res, const Core::Vector3Float& eulerAngles) const
    {
        const auto pitchQuat = simd_quaternion(eulerAngles.x, simd_float3{1.0f, 0.0f, 0.0f});
        const auto yawQuat = simd_quaternion(eulerAngles.y, simd_float3{0.0f, 1.0f, 0.0f});
        const auto rollQuat = simd_quaternion(eulerAngles.z, simd_float3{0.0f, 0.0f, 1.0f});

        const auto r = simd_mul(simd_mul(yawQuat, pitchQuat), rollQuat);
        std::memcpy(&res, &r, sizeof(res));
    }

    void Math::Multiply(Core::Quaternion& res, const Core::Quaternion& q1, const Core::Quaternion& q2) const
    {
        const auto& simdQ1 = reinterpret_cast<const simd_quatf&>(q1);
        const auto& simdQ2 = reinterpret_cast<const simd_quatf&>(q2);

        const auto r = simd_mul(simdQ1, simdQ2);
        std::memcpy(&res, &r, sizeof(res));
    }

    /*void Math::CalculateLocalToWorldSpaceMatrix(Core::Matrix4x4& res, const std::shared_ptr<const Core::Node>& node) const
    {
        auto m = TRSInternalTransposed(node->localTransform);
        auto currentNode = node->GetParent();

        while (currentNode)
        {
            m = DirectX::XMMatrixMultiply(m, TRSInternalTransposed(currentNode->localTransform));
            currentNode = currentNode->GetParent();
        }

        DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&res), DirectX::XMMatrixTranspose(m));
    }
*/
}

