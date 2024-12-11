#pragma once
#include <d3d12.h>
#include <wrl/client.h>

namespace MMPEngine::Backend::Dx12
{
	class Job
	{
	protected:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;
	};
}