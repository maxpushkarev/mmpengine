#include <Backend/Dx12/Math.hpp>
#include <DirectXMath.h>

namespace MMPEngine::Backend::Dx12
{
	std::float_t Math::Dot(const Core::Vector3Float& v1, const Core::Vector3Float& v2) const
	{
		const auto dot = DirectX::XMVector3Dot(
			DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&v1)),
			DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&v2))
		);

		std::float_t res = 0;
		DirectX::XMStoreFloat(&res, dot);

		return res;
	}

	void Math::Cross(Core::Vector3Float& res, const Core::Vector3Float& v1, const Core::Vector3Float& v2) const
	{
		const auto cross = DirectX::XMVector3Cross(
			DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&v1)),
			DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&v2))
		);
		DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&res), cross);
	}

	std::float_t Math::Magnitude(const Core::Vector3Float& v) const
	{
		const auto length = DirectX::XMVector3Length(
			DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&v))
		);

		std::float_t res = 0;
		DirectX::XMStoreFloat(&res, length);
		return res;
	}

	void Math::Normalize(Core::Vector3Float& v) const
	{
		const auto nrm = DirectX::XMVector3Normalize(
			DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&v))
		);
		DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&v), nrm);
	}

	std::float_t Math::SquaredMagnitude(const Core::Vector3Float& v) const
	{
		const auto sqr = DirectX::XMVector3LengthSq(
			DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&v))
		);

		std::float_t res = 0;
		DirectX::XMStoreFloat(&res, sqr);
		return res;
	}

	void Math::Multiply(Core::Matrix4x4& res, const Core::Matrix4x4& m1, const Core::Matrix4x4& m2) const
	{
		const auto m1Loaded = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&m1));
		const auto m2Loaded = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&m2));
		const auto r = DirectX::XMMatrixMultiply(m1Loaded, m2Loaded);
		DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&res), r);
	}

	void Math::Multiply(Core::Vector4Float& res, const Core::Matrix4x4& m, const Core::Vector4Float& v) const
	{
		const auto vLoaded = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&v));
		const auto mLoaded = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&m));
		const auto r = DirectX::XMVector4Transform(vLoaded, DirectX::XMMatrixTranspose(mLoaded));
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&res), r);
	}

	std::float_t Math::Determinant(const Core::Matrix4x4& m) const
	{
		std::float_t res = 0.0f;

		const auto mLoaded = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&m));
		const auto det = DirectX::XMMatrixDeterminant(mLoaded);
		DirectX::XMStoreFloat(&res, det);

		return res;
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
}