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

    /*std::float_t Math::Determinant(const Core::Matrix4x4& m) const
    {
        std::float_t res = 0.0f;

        const auto mLoaded = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&m));
        const auto det = DirectX::XMMatrixDeterminant(mLoaded);
        DirectX::XMStoreFloat(&res, det);

        return res;
    }*/

    /*

    DirectX::XMMATRIX XM_CALLCONV Math::TRSInternalTransposed(const Core::Transform& transform)
    {
        const auto position = DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&transform.position));
        const auto scale = DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&transform.scale));
        const auto rotation = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&transform.rotation));
        const auto orig = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        return DirectX::XMMatrixTransformation(orig, orig, scale, orig, rotation, position);
    }

    void Math::Inverse(Core::Matrix4x4& res, const Core::Matrix4x4& m) const
    {
        const auto mLoaded = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&m));
        const auto inv = DirectX::XMMatrixInverse(nullptr, mLoaded);
        DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&res), inv);
    }

    void Math::Transpose(Core::Matrix4x4& res, const Core::Matrix4x4& m) const
    {
        const auto mLoaded = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&m));
        const auto transposed = DirectX::XMMatrixTranspose(mLoaded);
        DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&res), transposed);
    }

    void Math::InverseTranspose(Core::Matrix4x4& res, const Core::Matrix4x4& m) const
    {
        const auto mLoaded = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&m));
        const auto inv = DirectX::XMMatrixInverse(nullptr, mLoaded);
        DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&res), DirectX::XMMatrixTranspose(inv));
    }

    void Math::MultiplyMatrixAndPoint(Core::Vector3Float& res, const Core::Matrix4x4& m, const Core::Vector3Float& p) const
    {
        const auto pLoaded = DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&p));
        const auto mLoaded = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&m));
        const auto r = DirectX::XMVector3TransformCoord(pLoaded, DirectX::XMMatrixTranspose(mLoaded));
        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&res), r);
    }

    void Math::MultiplyMatrixAndVector(Core::Vector3Float& res, const Core::Matrix4x4& m, const Core::Vector3Float& v) const
    {
        const auto vLoaded = DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&v));

        const auto t = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&m)));
        DirectX::XMFLOAT3X3 tmp {};
        DirectX::XMStoreFloat3x3(&tmp, t);

        const auto r = DirectX::XMVector3Transform(vLoaded, DirectX::XMLoadFloat3x3(&tmp));
        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&res), r);
    }

    void Math::RotationAroundAxis(Core::Quaternion& res, const Core::Vector3Float& v, std::float_t rad) const
    {
        const auto nrm = DirectX::XMVector3Normalize(
            DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&v))
        );
        const auto q = DirectX::XMQuaternionRotationNormal(nrm, rad);
        DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&res), q);
    }

    std::float_t Math::Dot(const Core::Quaternion& q1, const Core::Quaternion& q2) const
    {
        const auto q1Loaded = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&q1));
        const auto q2Loaded = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&q2));

        std::float_t res = 0.0f;
        const auto dot = DirectX::XMQuaternionDot(q1Loaded, q2Loaded);
        DirectX::XMStoreFloat(&res, dot);

        return res;
    }

    void Math::Normalize(Core::Quaternion& q) const
    {
        const auto unorm = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&q));
        const auto norm = DirectX::XMQuaternionNormalize(unorm);
        DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&q), norm);
    }

    void Math::Inverse(Core::Quaternion& res, const Core::Quaternion& q) const
    {
        const auto qLoaded = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&q));
        const auto inv = DirectX::XMQuaternionInverse(qLoaded);
        DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&res), inv);
    }

    void Math::RotationFromEuler(Core::Quaternion& res, const Core::Vector3Float& eulerAngles) const
    {
        const auto eulerLoaded = DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&eulerAngles));
        const auto q = DirectX::XMQuaternionRotationRollPitchYawFromVector(eulerLoaded);
        DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&res), q);
    }

    void Math::Multiply(Core::Quaternion& res, const Core::Quaternion& q1, const Core::Quaternion& q2) const
    {
        const auto q1Loaded = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&q1));
        const auto q2Loaded = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&q2));

        DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&res), DirectX::XMQuaternionMultiply(q2Loaded, q1Loaded));
    }


    void Math::CalculateLocalToWorldSpaceMatrix(Core::Matrix4x4& res, const std::shared_ptr<const Core::Node>& node) const
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

