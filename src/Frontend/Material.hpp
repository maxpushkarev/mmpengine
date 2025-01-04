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
		Material(const std::shared_ptr<std::enable_if_t<std::is_same_v<Core::ComputeMaterial, T>, Core::GlobalContext>>& globalContext, Core::BaseMaterial::Parameters&& params, const std::shared_ptr<Core::ComputeShader>& computeShader);
		template<typename T = TCoreMaterial>
		Material(const std::shared_ptr<std::enable_if_t<std::is_same_v<Core::MeshMaterial, T>, Core::GlobalContext>>& globalContext, 
			const Core::MeshMaterial::Settings& settings,
			Core::BaseMaterial::Parameters&& params,
			const std::shared_ptr<Core::VertexShader>& vs,
			const std::shared_ptr<Core::PixelShader>& ps);
	public:
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateTaskForApply() override;
		std::shared_ptr<Core::BaseMaterial> GetUnderlyingMaterial() override;
		const Core::BaseMaterial::Parameters& GetParameters() const override;
	protected:
		std::shared_ptr<TCoreMaterial> _impl;
	};


	class ComputeMaterial final : public Material<Core::ComputeMaterial>
	{
	public:
		ComputeMaterial(const std::shared_ptr<Core::GlobalContext>& globalContext, Core::BaseMaterial::Parameters&& params, const std::shared_ptr<Core::ComputeShader>& computeShader);
	};


	class MeshMaterial final : public Material<Core::MeshMaterial>
	{
	public:
		MeshMaterial(
			const std::shared_ptr<Core::GlobalContext>& globalContext,
			const Core::MeshMaterial::Settings& settings,
			Core::BaseMaterial::Parameters&& params,
			const std::shared_ptr<Core::VertexShader>& vs, 
			const std::shared_ptr<Core::PixelShader>& ps
		);
	};

	template<typename TCoreMaterial>
	template<typename T>
	inline Material<TCoreMaterial>::Material(const std::shared_ptr<std::enable_if_t<std::is_same_v<Core::ComputeMaterial, T>, Core::GlobalContext>>& globalContext, Core::BaseMaterial::Parameters&& params, const std::shared_ptr<Core::ComputeShader>& computeShader)
		: TCoreMaterial(std::move(params), computeShader)
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::ComputeMaterial>(std::move(this->_params), computeShader);
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
		Core::BaseMaterial::Parameters&& params,
		const std::shared_ptr<Core::VertexShader>& vs, 
		const std::shared_ptr<Core::PixelShader>& ps) : TCoreMaterial(settings, std::move(params), vs, ps)
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::MeshMaterial>(settings, std::move(this->_params), vs, ps);
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
	inline std::shared_ptr<Core::BaseMaterial> Material<TCoreMaterial>::GetUnderlyingMaterial()
	{
		return _impl->GetUnderlyingMaterial();
	}

	template<typename TCoreMaterial>
	inline const Core::BaseMaterial::Parameters& Material<TCoreMaterial>::GetParameters() const
	{
		return _impl->GetParameters();
	}
}