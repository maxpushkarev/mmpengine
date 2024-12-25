#pragma once
#include <Core/Material.hpp>
#include <Core/Context.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Material.hpp>
#endif

namespace MMPEngine::Frontend
{
	template<typename TCoreMaterial>
	class Material : public TCoreMaterial
	{
	protected:
		template<typename T = TCoreMaterial>
		Material(const std::shared_ptr<std::enable_if_t<std::is_same_v<Core::ComputeMaterial, T>, Core::GlobalContext>>& globalContext, const std::shared_ptr<Core::ComputeShader>& computeShader);
		template<typename T = TCoreMaterial>
		Material(const std::shared_ptr<std::enable_if_t<std::is_same_v<Core::MeshMaterial, T>, Core::GlobalContext>>& globalContext, 
			const Core::MeshMaterial::Settings& settings,
			const std::shared_ptr<Core::VertexShader>& vs,
			const std::shared_ptr<Core::PixelShader>& ps);
	public:
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateTaskForApply() override;
		std::shared_ptr<Core::BaseMaterial> GetUnderlyingMaterial() override;
		void SetParameters(Core::BaseMaterial::Parameters&& params) override;
		const Core::BaseMaterial::Parameters& GetParameters() const;
	protected:
		std::shared_ptr<Core::BaseTask> CreateTaskForBakeParametersInternal() override;
		std::shared_ptr<TCoreMaterial> _impl;
	};


	class ComputeMaterial final : public Material<Core::ComputeMaterial>
	{
	public:
		ComputeMaterial(const std::shared_ptr<Core::GlobalContext>& globalContext, const std::shared_ptr<Core::ComputeShader>& computeShader);
	};


	class MeshMaterial final : public Material<Core::MeshMaterial>
	{
	public:
		MeshMaterial(
			const std::shared_ptr<Core::GlobalContext>& globalContext,
			const Core::MeshMaterial::Settings& settings,
			const std::shared_ptr<Core::VertexShader>& vs, 
			const std::shared_ptr<Core::PixelShader>& ps
		);
	};

	template<typename TCoreMaterial>
	template<typename T>
	inline Material<TCoreMaterial>::Material(const std::shared_ptr<std::enable_if_t<std::is_same_v<Core::ComputeMaterial, T>, Core::GlobalContext>>& globalContext, const std::shared_ptr<Core::ComputeShader>& computeShader)
		: TCoreMaterial(computeShader)
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::ComputeMaterial>(computeShader);
#else
			throw Core::UnsupportedException("unable to create compute material for DX12 backend");
#endif
		}
	}

	template<typename TCoreMaterial>
	template<typename T>
	inline Material<TCoreMaterial>::Material(
		const std::shared_ptr<std::enable_if_t<std::is_same_v<Core::MeshMaterial, T>, Core::GlobalContext>>& globalContext, 
		const Core::MeshMaterial::Settings& settings, 
		const std::shared_ptr<Core::VertexShader>& vs, 
		const std::shared_ptr<Core::PixelShader>& ps) : TCoreMaterial(settings, vs, ps)
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::MeshMaterial>(settings, vs, ps);
#else
			throw Core::UnsupportedException("unable to create mesh material for DX12 backend");
#endif
		}
	}

	template<typename TCoreMaterial>
	inline std::shared_ptr<Core::BaseTask> Material<TCoreMaterial>::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();
	}

	template<typename TCoreMaterial>
	inline std::shared_ptr<Core::BaseTask> Material<TCoreMaterial>::CreateTaskForApply()
	{
		return _impl->CreateTaskForApply();
	}

	template<typename TCoreMaterial>
	inline std::shared_ptr<Core::BaseTask> Material<TCoreMaterial>::CreateTaskForBakeParametersInternal()
	{
		return _impl->CreateTaskForBakeParameters();
	}

	template<typename TCoreMaterial>
	inline std::shared_ptr<Core::BaseMaterial> Material<TCoreMaterial>::GetUnderlyingMaterial()
	{
		return _impl->GetUnderlyingMaterial();
	}

	template<typename TCoreMaterial>
	inline void Material<TCoreMaterial>::SetParameters(Core::BaseMaterial::Parameters&& params)
	{
		_impl->SetParameters(std::move(params));
		this->_bakedParams = false;
	}

	template<typename TCoreMaterial>
	inline const Core::BaseMaterial::Parameters& Material<TCoreMaterial>::GetParameters() const
	{
		return _impl->GetParameters();
	}
}