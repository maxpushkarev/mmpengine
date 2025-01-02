#include <Backend/Dx12/Texture.hpp>
#include <Backend/Dx12/d3dx12.h>

namespace MMPEngine::Backend::Dx12
{
	DepthStencilTargetTexture::DepthStencilTargetTexture(const Settings& settings) : Core::DepthStencilTargetTexture(settings)
	{
	}

	std::shared_ptr<Core::BaseTask> DepthStencilTargetTexture::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<DepthStencilTargetTexture>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	DXGI_FORMAT DepthStencilTargetTexture::GetResourceFormat() const
	{
		switch (_settings.format)
		{
		case Settings::Format::Depth24_Stencil8:
			return DXGI_FORMAT_R24G8_TYPELESS;
		case Settings::Format::Depth32:
			return DXGI_FORMAT_R32_TYPELESS;
		case Settings::Format::Depth16:
			return DXGI_FORMAT_R16_TYPELESS;
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	DXGI_FORMAT DepthStencilTargetTexture::GetDSVFormat() const
	{
		switch (_settings.format)
		{
		case Settings::Format::Depth24_Stencil8:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case Settings::Format::Depth32:
			return DXGI_FORMAT_D32_FLOAT;
		case Settings::Format::Depth16:
			return DXGI_FORMAT_D16_UNORM;
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	DepthStencilTargetTexture::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task(ctx)
	{
	}

	void DepthStencilTargetTexture::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto dsTex = GetTaskContext()->entity;
		const auto gc = _specificGlobalContext;

		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = dsTex->_settings.base.size.x;
		depthStencilDesc.Height = dsTex->_settings.base.size.y;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.Format = dsTex->GetResourceFormat();
		depthStencilDesc.SampleDesc = {dsTex->_settings.base.antialiasing == Core::TargetTexture::Settings::Antialiasing::MSAA_0 ? 1 : (static_cast<std::uint32_t>(dsTex->_settings.base.antialiasing)), 0};
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		const D3D12_CLEAR_VALUE* optClearPtr = nullptr;

		if(dsTex->_settings.clearValue.has_value())
		{
			D3D12_CLEAR_VALUE optClear;
			optClear.Format = dsTex->GetDSVFormat();
			optClear.DepthStencil.Depth = std::get<std::float_t>(dsTex->_settings.clearValue.value());
			optClear.DepthStencil.Stencil = std::get<std::uint8_t>(dsTex->_settings.clearValue.value());

			optClearPtr = &optClear;
		}

		Microsoft::WRL::ComPtr<ID3D12Resource> nativeResource;

		gc->device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			optClearPtr,
			IID_PPV_ARGS(nativeResource.GetAddressOf()));

		dsTex->SetNativeResource(nativeResource, 0);
	}

}