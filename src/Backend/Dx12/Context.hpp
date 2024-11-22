#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include <Core/Context.hpp>
#include <wrl/client.h>

namespace MMPEngine::Backend::Dx12
{
	class AppContext : public Core::AppContext
	{
	public:
		AppContext(const Core::AppContext::Settings& s);

		Microsoft::WRL::ComPtr<ID3D12Device> device;
		Microsoft::WRL::ComPtr<IDXGIFactory> factory;
	};
}
