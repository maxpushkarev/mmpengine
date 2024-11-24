#include <Backend/Dx12/Entity.hpp>

namespace MMPEngine::Backend::Dx12
{
	BaseEntity::BaseEntity(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource) : _nativeResource(nativeResource)
	{
		SetUpNativeResourceName();
	}

	BaseEntity::BaseEntity(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource, std::string_view name) : Core::BaseEntity(name), _nativeResource(nativeResource)
	{
		SetUpNativeResourceName();
	}

	void BaseEntity::SetUpNativeResourceName()
	{
		const auto str = GetName();
		std::wstring wName(std::begin(str), std::end(str));
		_nativeResource->SetName(wName.c_str());
	}

}