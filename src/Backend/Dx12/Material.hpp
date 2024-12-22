#pragma once
#include <functional>
#include <cassert>
#include <Core/Material.hpp>
#include <Backend/Dx12/Task.hpp>
#include <Backend/Dx12/Stream.hpp>
#include <Backend/Dx12/Buffer.hpp>
#include <Backend/Dx12/Texture.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Material
	{
	public:
		Material();
		Material(const Material&) = delete;
		Material(Material&&) noexcept = delete;
		Material& operator=(const Material&) = delete;
		Material& operator=(Material&&) noexcept = delete;
		virtual ~Material();

		Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() const;

	protected:

		class ApplyMaterialTaskContext final : public Core::TaskContext
		{
		public:
			std::shared_ptr<Material> materialPtr;
		};

		class ApplyParametersTask : public Task<ApplyMaterialTaskContext>
		{
		private:
			class SwitchState final : public Task<ApplyMaterialTaskContext>
			{
			public:
				SwitchState(const std::shared_ptr<ApplyMaterialTaskContext>& context);
				void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

			class Apply final : public Task<ApplyMaterialTaskContext>
			{
			public:
				Apply(const std::shared_ptr<ApplyMaterialTaskContext>& context);
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

		public:
			ApplyParametersTask(const std::shared_ptr<ApplyMaterialTaskContext>& context);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;

		private:
			std::shared_ptr<Core::BaseTask> _switchState;
			std::shared_ptr<Core::BaseTask> _apply;
		};
		
		std::vector<std::function<void(const std::shared_ptr<StreamContext>& streamContext)>> _applyParametersCallbacks;

		void UpdateRootSignatureAndSwitchTasks(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params);

	private:
		void SwitchParametersStates(const std::shared_ptr<Core::BaseStream>& stream);
		void ApplyParameters(const std::shared_ptr<StreamContext>& streamContext);

		Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;
		std::vector<std::shared_ptr<Core::BaseTask>> _switchStateTasks;
	};

	template<class TCoreMaterial>
	class MaterialImpl : public Material
	{
	protected:

		class BakeParametersTaskContext final : public Core::TaskContext
		{
		public:
			std::shared_ptr<MaterialImpl> materialImplPtr;
			const Core::BaseMaterial::Parameters* params = nullptr;
		};

		class BakeParametersTask : public Task<BakeParametersTaskContext>
		{
		public:
			BakeParametersTask(const std::shared_ptr<BakeParametersTaskContext>& context);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		void OnParametersBake(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params);
	};

	class ComputeMaterial final : public Core::ComputeMaterial, public MaterialImpl<Core::ComputeMaterial>
	{
	public:
		ComputeMaterial(const std::shared_ptr<Core::ComputeShader>& computeShader);
		std::shared_ptr<Core::BaseTask> CreateTaskForApply() override;
	protected:
		std::shared_ptr<Core::BaseTask> CreateTaskForBakeParametersInternal() override;
	};

	template<class TCoreMaterial>
	inline void MaterialImpl<TCoreMaterial>::OnParametersBake(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params)
	{
		_applyParametersCallbacks.clear();

		this->UpdateRootSignatureAndSwitchTasks(globalContext, params);

		const auto& allParams = params.GetAll();

		for(std::size_t i = 0; i < allParams.size(); ++i)
		{
			const auto& parameterEntry = allParams.at(i);
			const auto index = static_cast<std::uint32_t>(i);

			if constexpr (std::is_same_v<TCoreMaterial, Core::ComputeMaterial>)
			{
				if(std::holds_alternative<Core::BaseMaterial::Parameters::Buffer>(parameterEntry.settings))
				{
					const auto bufferSettings = std::get<Core::BaseMaterial::Parameters::Buffer>(parameterEntry.settings);
					const auto coreBuffer = std::dynamic_pointer_cast<Core::Buffer>(parameterEntry.entity);
					assert(coreBuffer);


					switch (bufferSettings.type)
					{
						case Core::BaseMaterial::Parameters::Buffer::Type::UnorderedAccess:
							{
								const auto nativeBuffer = std::dynamic_pointer_cast<Dx12::ResourceEntity>(coreBuffer->GetUnderlyingBuffer());
								assert(nativeBuffer);
								_applyParametersCallbacks.emplace_back([nativeBuffer, index](const auto& ctx)
								{
									ctx->PopulateCommandsInList()->SetComputeRootDescriptorTable(static_cast<std::uint32_t>(index), nativeBuffer->GetShaderVisibleDescriptorHandle()->GetGPUDescriptorHandle());
								});
								break;
							}
						case Core::BaseMaterial::Parameters::Buffer::Type::Uniform:
							{
								const auto nativeBuffer = std::dynamic_pointer_cast<Dx12::ResourceEntity>(coreBuffer->GetUnderlyingBuffer());
								assert(nativeBuffer);
								_applyParametersCallbacks.emplace_back([nativeBuffer, index](const auto& ctx)
								{
									ctx->PopulateCommandsInList()->SetComputeRootConstantBufferView(static_cast<std::uint32_t>(index), nativeBuffer->GetNativeResource()->GetGPUVirtualAddress());
								});
								break;
							}
						case Core::BaseMaterial::Parameters::Buffer::Type::ReadonlyAccess:
							{
								const auto nativeBuffer = std::dynamic_pointer_cast<Dx12::ResourceEntity>(coreBuffer->GetUnderlyingBuffer());
								assert(nativeBuffer);
								_applyParametersCallbacks.emplace_back([nativeBuffer, index](const auto& ctx)
								{
									ctx->PopulateCommandsInList()->SetComputeRootShaderResourceView(static_cast<std::uint32_t>(index), nativeBuffer->GetNativeResource()->GetGPUVirtualAddress());
								});
								break;
							}
					}

					continue;	
				}

				if (std::holds_alternative<Core::BaseMaterial::Parameters::Texture>(parameterEntry.settings))
				{
					continue;
				}

				throw Core::UnsupportedException("unsupported dx12 entity type in material");
			}
			else if constexpr (std::is_base_of_v<Core::RenderingMaterial, TCoreMaterial>)
			{
				//TODO: populate graphics commands
			}
			else
			{
				throw Core::UnsupportedException("unsupported dx12 material type");
			}
		}
	}

	template<class TCoreMaterial>
	inline MaterialImpl<TCoreMaterial>::BakeParametersTask::BakeParametersTask(const std::shared_ptr<BakeParametersTaskContext>& context) : Task<BakeParametersTaskContext>(context)
	{
	}


	template<class TCoreMaterial>
	inline void MaterialImpl<TCoreMaterial>::BakeParametersTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task<BakeParametersTaskContext>::Run(stream);
		if (const auto tc = this->GetTaskContext() ; const auto matImpl = tc->materialImplPtr)
		{
			matImpl->OnParametersBake(this->_specificGlobalContext, *tc->params);
		}
	}
}
