#include <Backend/Dx12/Material.hpp>
#include <Backend/Dx12/d3dx12.h>

namespace MMPEngine::Backend::Dx12
{
	Material::Material() = default;
	Material::~Material() = default;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> Material::GetRootSignature() const
	{
		return _rootSignature;
	}

	void Material::ApplyParameters(const std::shared_ptr<StreamContext>& streamContext)
	{
		for (const auto& fn : _applyParametersCallbacks)
		{
			fn(streamContext);
		}
	}

    void Material::UpdateRootSignatureAndSwitchTasks(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params)
    {
		_rootSignature.Reset();
		_switchStateTasks.clear();

		const auto& allParams = params.GetAll();

		D3D12_ROOT_SIGNATURE_DESC rootSignature{};

		std::vector rootParameters (allParams.size(), CD3DX12_ROOT_PARAMETER{});
		std::unordered_map<D3D12_DESCRIPTOR_RANGE_TYPE, std::uint32_t> baseRegisters {};

		const auto calculateBaseRegisterFn = [&baseRegisters](D3D12_DESCRIPTOR_RANGE_TYPE type) -> auto
		{
			if(baseRegisters.find(type) == baseRegisters.cend())
			{
				baseRegisters[type] = 0;
			}
			else
			{
				baseRegisters[type]++;
			}

			return baseRegisters[type];
		};


		for (std::size_t i = 0; i < allParams.size(); ++i)
		{
			const auto& parameterEntry = allParams.at(i);
			const auto index = static_cast<std::uint32_t>(i);

			const auto switchStateTaskContext = std::make_shared<Dx12::ResourceEntity::SwitchStateTaskContext>();

			if (std::holds_alternative<Core::BaseMaterial::Parameters::Buffer>(parameterEntry.settings))
			{
				const auto bufferSettings = std::get<Core::BaseMaterial::Parameters::Buffer>(parameterEntry.settings);
				const auto coreBuffer = std::dynamic_pointer_cast<Core::Buffer>(parameterEntry.entity);
				assert(coreBuffer);
				const auto nativeBuffer = std::dynamic_pointer_cast<Dx12::ResourceEntity>(coreBuffer->GetUnderlyingBuffer());
				assert(nativeBuffer);

				switchStateTaskContext->entity = nativeBuffer;

				switch (bufferSettings.type)
				{
					case Core::BaseMaterial::Parameters::Buffer::Type::UnorderedAccess:
						{
							switchStateTaskContext->nextStateMask = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

							D3D12_DESCRIPTOR_RANGE range{};
							range.NumDescriptors = 1;
							range.OffsetInDescriptorsFromTableStart = 0;
							range.RegisterSpace = 0;

							range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
							range.BaseShaderRegister = calculateBaseRegisterFn(D3D12_DESCRIPTOR_RANGE_TYPE_UAV);

							rootParameters[index].InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_ALL);

							break;
						}
					case Core::BaseMaterial::Parameters::Buffer::Type::UniformConstants:
						{
							switchStateTaskContext->nextStateMask = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
							rootParameters[index].InitAsConstantBufferView(calculateBaseRegisterFn(D3D12_DESCRIPTOR_RANGE_TYPE_CBV), 0, D3D12_SHADER_VISIBILITY_ALL);
							break;
						}
					case Core::BaseMaterial::Parameters::Buffer::Type::ReadonlyAccess:
						{
							switchStateTaskContext->nextStateMask = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
							rootParameters[index].InitAsShaderResourceView(calculateBaseRegisterFn(D3D12_DESCRIPTOR_RANGE_TYPE_SRV), 0, D3D12_SHADER_VISIBILITY_ALL);
							break;
						}
				}

				_switchStateTasks.emplace_back(std::make_shared<Dx12::ResourceEntity::SwitchStateTask>(switchStateTaskContext));
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
		rootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

		Microsoft::WRL::ComPtr<ID3DBlob> blob;
		Microsoft::WRL::ComPtr<ID3DBlob> error;

		D3D12SerializeRootSignature(&rootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
		assert(error == nullptr);
		globalContext->device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(_rootSignature.GetAddressOf()));
		assert(_rootSignature != nullptr);
    }

    void Material::SwitchParametersStates(const std::shared_ptr<Core::BaseStream>& stream)
	{
		for (const auto& task : _switchStateTasks)
		{
			stream->Schedule(task);
		}
	}


	Material::ApplyParametersTask::ApplyParametersTask(const std::shared_ptr<ApplyMaterialTaskContext>& context) : Task<ApplyMaterialTaskContext>(context)
	{
		_switchState = std::make_shared<SwitchState>(context);
		_apply = std::make_shared<Apply>(context);
	}

	void Material::ApplyParametersTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		stream->Schedule(_switchState);
		stream->Schedule(_apply);
	}


	Material::ApplyParametersTask::SwitchState::SwitchState(const std::shared_ptr<ApplyMaterialTaskContext>& context) : Task<ApplyMaterialTaskContext>(context)
	{
	}

	void Material::ApplyParametersTask::SwitchState::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		if (const auto mat = this->GetTaskContext()->materialPtr)
		{
			mat->SwitchParametersStates(stream);
		}
	}

	Material::ApplyParametersTask::Apply::Apply(const std::shared_ptr<ApplyMaterialTaskContext>& context) : Task<ApplyMaterialTaskContext>(context)
	{
	}

	void Material::ApplyParametersTask::Apply::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		if (const auto mat = this->GetTaskContext()->materialPtr; const auto sc = _specificStreamContext)
		{
			mat->ApplyParameters(sc);
		}
	}

	ComputeMaterial::ComputeMaterial(const std::shared_ptr<Core::ComputeShader>& computeShader) : Core::ComputeMaterial(computeShader)
	{
	}

	std::shared_ptr<Core::BaseTask> ComputeMaterial::CreateTaskForApply()
	{
		const auto ctx = std::make_shared<ApplyMaterialTaskContext>();
		ctx->materialPtr = std::dynamic_pointer_cast<MaterialImpl<Core::ComputeMaterial>>(shared_from_this());
		return std::make_shared<ApplyParametersTask>(ctx);
	}

	std::shared_ptr<Core::BaseTask> ComputeMaterial::CreateTaskForBakeParametersInternal()
	{
		const auto ctx = std::make_shared<ParametersUpdatedTaskContext>();
		ctx->materialImplPtr = std::dynamic_pointer_cast<MaterialImpl<Core::ComputeMaterial>>(shared_from_this());
		ctx->params = &_params;
		return std::make_shared<ParametersUpdatedTask>(ctx);
	}

}
