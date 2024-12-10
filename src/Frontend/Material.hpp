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
		Material(const std::shared_ptr<std::enable_if_t<std::is_same_v<Core::ComputeMaterial, T>, Core::AppContext>>& appContext, const std::shared_ptr<Core::ComputeShader>& computeShader);
	public:
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateTaskForApply() override;
		std::shared_ptr<Core::BaseMaterial> GetUnderlyingMaterial() override;
	protected:
		std::shared_ptr<Core::BaseTask> CreateTaskForUpdateParametersInternal() override;
		std::shared_ptr<TCoreMaterial> _impl;
	};


	class ComputeMaterial final : public Material<Core::ComputeMaterial>
	{
	public:
		ComputeMaterial(const std::shared_ptr<Core::AppContext>& appContext, const std::shared_ptr<Core::ComputeShader>& computeShader);
	};


	template<typename TCoreMaterial>
	template<typename T>
	inline Material<TCoreMaterial>::Material(const std::shared_ptr<std::enable_if_t<std::is_same_v<Core::ComputeMaterial, T>, Core::AppContext>>& appContext, const std::shared_ptr<Core::ComputeShader>& computeShader)
		: TCoreMaterial(computeShader)
	{
		if (appContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::ComputeMaterial>(computeShader);
#else
			throw Core::UnsupportedException("unable to create compute material for DX12 backend");
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
	inline std::shared_ptr<Core::BaseTask> Material<TCoreMaterial>::CreateTaskForUpdateParametersInternal()
	{
		return _impl->CreateTaskForUpdateParameters(std::move(this->_params));
	}

	template<typename TCoreMaterial>
	inline std::shared_ptr<Core::BaseMaterial> Material<TCoreMaterial>::GetUnderlyingMaterial()
	{
		return _impl->GetUnderlyingMaterial();
	}
}