#pragma once
#include <d3d12.h>
#include <Core/Entity.hpp>
#include <wrl/client.h>

namespace MMPEngine::Backend::Dx12
{
	class BaseEntity : Core::BaseEntity
	{
	protected:
		BaseEntity(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource, std::string_view name);
		BaseEntity(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource);
	private:
		void SetUpNativeResourceName();
	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> _nativeResource;
	};
}
