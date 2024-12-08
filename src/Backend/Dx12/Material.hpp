#pragma once
#include <functional>
#include <Core/Material.hpp>
#include <Backend/Dx12/Stream.hpp>

namespace MMPEngine::Backend::Dx12
{
	template<class TCoreMaterial>
	class MaterialImpl
	{
	protected:
		void OnParametersUpdated(const Core::BaseMaterial::Parameters& params);
		void ApplyParameters(const std::shared_ptr<Stream>& stream);
	private:
		std::unordered_map<std::string_view, std::function<void(const std::shared_ptr<Stream>& stream)>> _applyParameters;
	};

	class ComputeMaterial final : public Core::ComputeMaterial, public MaterialImpl<Core::ComputeMaterial>
	{
	public:
		ComputeMaterial(const std::shared_ptr<Core::ComputeShader>& computeShader);
		std::shared_ptr<Core::BaseTask> CreateTaskForApply() override;
		std::shared_ptr<Core::BaseTask> CreateTaskForUpdateParametersInternal() override;
	};

	template<class TCoreMaterial>
	inline void MaterialImpl<TCoreMaterial>::OnParametersUpdated(const Core::BaseMaterial::Parameters& params)
	{
		_applyParameters.clear();

		const auto& allParams = params.GetAll();

		for(std::size_t i = 0; i < allParams.size(); ++i)
		{
			const auto& p = allParams.at(i);

			if constexpr (std::is_same_v<TCoreMaterial, Core::ComputeMaterial>)
			{
				//TODO: populate compute commands
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
	inline void MaterialImpl<TCoreMaterial>::ApplyParameters(const std::shared_ptr<Stream>& stream)
	{
		for(const auto& [_, fn] : _applyParameters)
		{
			fn(stream);
		}
	}
}
