#include <cassert>
#include <Backend/Dx12/Job.hpp>
#include <Backend/Dx12/d3dx12.h>
#include <Backend/Dx12/Buffer.hpp>

namespace MMPEngine::Backend::Dx12
{
	BaseJob::BaseJob() = default;
	BaseJob::~BaseJob() = default;

	void BaseJob::BakeMaterialParameters(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params, D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags)
	{
		_rootSignature.Reset();
		_switchMaterialParametersStateTasks.clear();

		const auto& allParams = params.GetAll();

		D3D12_ROOT_SIGNATURE_DESC rootSignature{};

		std::vector rootParameters(allParams.size(), CD3DX12_ROOT_PARAMETER{});
		std::unordered_map<D3D12_DESCRIPTOR_RANGE_TYPE, std::uint32_t> baseRegisters {};
		std::unordered_map<std::size_t, D3D12_DESCRIPTOR_RANGE> indexToDescriptorRangeMap {};

		const auto calculateBaseRegisterFn = [&baseRegisters](D3D12_DESCRIPTOR_RANGE_TYPE type) -> auto
		{
			if (baseRegisters.find(type) == baseRegisters.cend())
			{
				baseRegisters[type] = 0;
			}
			else
			{
				++baseRegisters[type];
			}

			return baseRegisters[type];
		};


		for (std::size_t i = 0; i < allParams.size(); ++i)
		{
			const auto& parameterEntry = allParams.at(i);
			const auto index = static_cast<std::uint32_t>(i);

			auto nextStateMask = D3D12_RESOURCE_STATE_COMMON;

			if (std::holds_alternative<Core::BaseMaterial::Parameters::Buffer>(parameterEntry.settings))
			{
				const auto bufferSettings = std::get<Core::BaseMaterial::Parameters::Buffer>(parameterEntry.settings);
				const auto coreBuffer = std::dynamic_pointer_cast<const Core::Buffer>(parameterEntry.entity);
				assert(coreBuffer);
				const auto nativeBuffer = std::dynamic_pointer_cast<Dx12::ResourceEntity>(coreBuffer->GetUnderlyingBuffer());
				assert(nativeBuffer);

				switch (bufferSettings.type)
				{
				case Core::BaseMaterial::Parameters::Buffer::Type::UnorderedAccess:
				{
					nextStateMask = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

					auto& range = indexToDescriptorRangeMap[index];
					range.NumDescriptors = 1;
					range.OffsetInDescriptorsFromTableStart = 0;
					range.RegisterSpace = 0;

					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
					range.BaseShaderRegister = calculateBaseRegisterFn(D3D12_DESCRIPTOR_RANGE_TYPE_UAV);

					rootParameters[index].InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_ALL);
				}
				break;
				case Core::BaseMaterial::Parameters::Buffer::Type::Uniform:
				{
					nextStateMask = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

					auto& range = indexToDescriptorRangeMap[index];
					range.NumDescriptors = 1;
					range.OffsetInDescriptorsFromTableStart = 0;
					range.RegisterSpace = 0;

					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					range.BaseShaderRegister = calculateBaseRegisterFn(D3D12_DESCRIPTOR_RANGE_TYPE_CBV);

					rootParameters[index].InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_ALL);

				}
				break;
				case Core::BaseMaterial::Parameters::Buffer::Type::ReadonlyAccess:
				{
					nextStateMask = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
					rootParameters[index].InitAsShaderResourceView(calculateBaseRegisterFn(D3D12_DESCRIPTOR_RANGE_TYPE_SRV), 0, D3D12_SHADER_VISIBILITY_ALL);
				}
				break;
				}

				_switchMaterialParametersStateTasks.emplace_back(nativeBuffer->CreateSwitchStateTask(nextStateMask));
				continue;
			}

			if (std::holds_alternative<Core::BaseMaterial::Parameters::Texture>(parameterEntry.settings))
			{
				continue;
			}

			throw Core::UnsupportedException("unsupported dx12 entity type in material");
		}

		rootSignature.NumParameters = static_cast<std::uint32_t>(rootParameters.size());
		rootSignature.pParameters = rootParameters.data();
		rootSignature.Flags = rootSignatureFlags;

		Microsoft::WRL::ComPtr<ID3DBlob> blob;
		Microsoft::WRL::ComPtr<ID3DBlob> error;

		D3D12SerializeRootSignature(&rootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
		assert(error == nullptr);
		globalContext->device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(_rootSignature.GetAddressOf()));
		assert(_rootSignature != nullptr);
	}


	BaseJob::ApplyParametersTask::ApplyParametersTask(const std::shared_ptr<ApplyParametersTaskContext>& context) : Task<ApplyParametersTaskContext>(context)
	{
		_switchState = std::make_shared<SwitchState>(context);
		_apply = std::make_shared<Apply>(context);
	}

	void BaseJob::ApplyParametersTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		stream->Schedule(_switchState);
		stream->Schedule(_apply);
	}


	BaseJob::ApplyParametersTask::SwitchState::SwitchState(const std::shared_ptr<ApplyParametersTaskContext>& context) : Task<ApplyParametersTaskContext>(context)
	{
	}

	void BaseJob::ApplyParametersTask::SwitchState::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		if (const auto job = this->GetTaskContext()->job)
		{
			for(const auto& sst : job->_switchMaterialParametersStateTasks)
			{
				stream->Schedule(sst);
			}
		}
	}

	BaseJob::ApplyParametersTask::Apply::Apply(const std::shared_ptr<ApplyParametersTaskContext>& context) : Task<ApplyParametersTaskContext>(context)
	{
	}

	void BaseJob::ApplyParametersTask::Apply::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		if (const auto job = this->GetTaskContext()->job; const auto sc = _specificStreamContext)
		{
			for(const auto& applyParameterCallback : job->_applyMaterialParametersCallbacks)
			{
				applyParameterCallback(sc);
			}
		}
	}
}