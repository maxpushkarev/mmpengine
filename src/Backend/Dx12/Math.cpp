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
		const auto length = DirectX::XMVector3Length(
			DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&v))
		);

		std::float_t res = 0;
		DirectX::XMStoreFloat(&res, length);
		return res;
	}
}