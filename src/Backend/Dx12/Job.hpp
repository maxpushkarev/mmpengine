#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cassert>
#include <Core/Material.hpp>
#include <Backend/Dx12/Context.hpp>
#include <Backend/Dx12/Task.hpp>
#include <Backend/Dx12/Buffer.hpp>
#include <Backend/Dx12/Entity.hpp>

namespace MMPEngine::Backend::Dx12
{
	class BaseJob
	{
	public:
		BaseJob(const BaseJob&) = delete;
		BaseJob(BaseJob&&) noexcept = delete;
		BaseJob& operator=(const BaseJob&) = delete;
		BaseJob& operator=(BaseJob&&) noexcept = delete;

	protected:
		BaseJob();
		virtual	~BaseJob();

		Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;

		std::vector<std::function<void(const std::shared_ptr<StreamContext>& streamContext)>> _applyMaterialParametersCallbacks;
		std::vector<std::shared_ptr<Core::BaseTask>> _switchMaterialParametersStateTasks;


		class ApplyParametersTaskContext final : public Core::TaskContext
		{
		public:
			std::shared_ptr<BaseJob> job;
		};

		class ApplyParametersTask : public Task<ApplyParametersTaskContext>
		{
		private:
			class SwitchState final : public Task<ApplyParametersTaskContext>
			{
			public:
				SwitchState(const std::shared_ptr<ApplyParametersTaskContext>& context);
				void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

			class Apply final : public Task<ApplyParametersTaskContext>
			{
			public:
				Apply(const std::shared_ptr<ApplyParametersTaskContext>& context);
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

		public:
			ApplyParametersTask(const std::shared_ptr<ApplyParametersTaskContext>& context);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;

		private:
			std::shared_ptr<Core::BaseTask> _switchState;
			std::shared_ptr<Core::BaseTask> _apply;
		};

		virtual void BakeMaterialParameters(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params);
	};

	template<typename TCoreMaterial>
	class Job : public BaseJob
	{
		static_assert(std::is_base_of_v<Core::BaseMaterial, TCoreMaterial>, "TCoreMaterial must be derived from Core::BaseMaterial");
	public:
		void BakeMaterialParameters(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params) override;
	};


	template<typename TCoreMaterial>
	inline void Job<TCoreMaterial>::BakeMaterialParameters(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params)
	{
		BaseJob::BakeMaterialParameters(globalContext, params);

		this->_applyMaterialParametersCallbacks.clear();

		const auto& allParams = params.GetAll();

		for (std::size_t i = 0; i < allParams.size(); ++i)
		{
			const auto& parameterEntry = allParams.at(i);
			const auto index = static_cast<std::uint32_t>(i);

			if constexpr (std::is_same_v<TCoreMaterial, Core::ComputeMaterial>)
			{
				if (std::holds_alternative<Core::BaseMaterial::Parameters::Buffer>(parameterEntry.settings))
				{
					const auto bufferSettings = std::get<Core::BaseMaterial::Parameters::Buffer>(parameterEntry.settings);
					const auto coreBuffer = std::dynamic_pointer_cast<const Core::Buffer>(parameterEntry.entity);
					assert(coreBuffer);
					const auto nativeBuffer = std::dynamic_pointer_cast<Dx12::BaseEntity>(coreBuffer->GetUnderlyingBuffer());
					assert(nativeBuffer);

					switch (bufferSettings.type)
					{
					case Core::BaseMaterial::Parameters::Buffer::Type::UnorderedAccess:
					{
						this->_applyMaterialParametersCallbacks.emplace_back([nativeBuffer, index](const auto& ctx)
							{
								ctx->PopulateCommandsInList()->SetComputeRootDescriptorTable(static_cast<std::uint32_t>(index), nativeBuffer->GetResourceDescriptorHandle()->GetGPUDescriptorHandle());
							});
					}
					break;
					case Core::BaseMaterial::Parameters::Buffer::Type::Uniform:
					{
						this->_applyMaterialParametersCallbacks.emplace_back([nativeBuffer, index](const auto& ctx)
							{
								ctx->PopulateCommandsInList()->SetComputeRootDescriptorTable(static_cast<std::uint32_t>(index), nativeBuffer->GetResourceDescriptorHandle()->GetGPUDescriptorHandle());
							});
					}
					break;
					case Core::BaseMaterial::Parameters::Buffer::Type::ReadonlyAccess:
					{
						this->_applyMaterialParametersCallbacks.emplace_back([nativeBuffer, index](const auto& ctx)
							{
								ctx->PopulateCommandsInList()->SetComputeRootShaderResourceView(static_cast<std::uint32_t>(index), nativeBuffer->GetNativeGPUAddressWithRequiredOffset());
							});
					}
					break;
					}

					continue;
				}

				if (std::holds_alternative<Core::BaseMaterial::Parameters::Texture>(parameterEntry.settings))
				{
					continue;
				}

				throw Core::UnsupportedException("unsupported dx12 entity type in material");
			}
			else if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
			{
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
						this->_applyMaterialParametersCallbacks.emplace_back([nativeBuffer, index](const auto& ctx)
							{
								ctx->PopulateCommandsInList()->SetGraphicsRootDescriptorTable(static_cast<std::uint32_t>(index), nativeBuffer->GetResourceDescriptorHandle()->GetGPUDescriptorHandle());
							});
					}
					break;
					case Core::BaseMaterial::Parameters::Buffer::Type::Uniform:
					{
						this->_applyMaterialParametersCallbacks.emplace_back([nativeBuffer, index](const auto& ctx)
							{
								ctx->PopulateCommandsInList()->SetGraphicsRootDescriptorTable(static_cast<std::uint32_t>(index), nativeBuffer->GetResourceDescriptorHandle()->GetGPUDescriptorHandle());
							});
					}
					break;
					case Core::BaseMaterial::Parameters::Buffer::Type::ReadonlyAccess:
					{
						this->_applyMaterialParametersCallbacks.emplace_back([nativeBuffer, index](const auto& ctx)
							{
								ctx->PopulateCommandsInList()->SetGraphicsRootShaderResourceView(static_cast<std::uint32_t>(index), nativeBuffer->GetNativeGPUAddressWithRequiredOffset());
							});
					}
					break;
					}

					continue;
				}

				if (std::holds_alternative<Core::BaseMaterial::Parameters::Texture>(parameterEntry.settings))
				{
					continue;
				}

				if (std::holds_alternative<Core::BaseMaterial::Parameters::StencilRef>(parameterEntry.settings))
				{
					const auto stencilRef = std::dynamic_pointer_cast<const Core::StencilRef>(parameterEntry.entity);
					assert(stencilRef);

					this->_applyMaterialParametersCallbacks.emplace_back([stencilRef](const auto& ctx)
						{
							ctx->PopulateCommandsInList()->OMSetStencilRef(static_cast<std::uint32_t>(stencilRef->value));
						});

					continue;
				}

				throw Core::UnsupportedException("unsupported dx12 entity type in material");
			}
			else
			{
				throw Core::UnsupportedException("unsupported dx12 material type");
			}
		}
	}
}