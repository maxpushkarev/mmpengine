#pragma once
#include <functional>
#include <Core/Material.hpp>
#include <Backend/Dx12/Task.hpp>
#include <Backend/Dx12/Stream.hpp>

namespace MMPEngine::Backend::Dx12
{
	template<class TCoreMaterial>
	class MaterialImpl
	{
	public:
		MaterialImpl();
		MaterialImpl(const MaterialImpl&) = delete;
		MaterialImpl(MaterialImpl&&) = delete;
		MaterialImpl& operator=(const MaterialImpl&) = delete;
		MaterialImpl& operator=(MaterialImpl&&) = delete;
		virtual ~MaterialImpl();
	protected:

		class ApplyMaterialTaskContext final : public Core::TaskContext
		{
		public:
			std::weak_ptr<MaterialImpl> materialImplPtr;
		};

		class ParametersUpdatedTaskContext final : public Core::TaskContext
		{
		public:
			std::weak_ptr<MaterialImpl> materialImplPtr;
			const Core::BaseMaterial::Parameters* params;
		};

		class ParametersUpdatedTask : public Task, public Core::TaskWithContext<ParametersUpdatedTaskContext>
		{
		public:
			ParametersUpdatedTask(const std::shared_ptr<ParametersUpdatedTaskContext>& context);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			void OnComplete(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class ApplyParametersTask : public Task, public Core::TaskWithContext<ApplyMaterialTaskContext>
		{
		public:
			ApplyParametersTask(const std::shared_ptr<ApplyMaterialTaskContext>& context);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			void OnComplete(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		void OnParametersUpdated(const Core::BaseMaterial::Parameters& params);
		void ApplyParameters(const std::shared_ptr<StreamContext>& stream);
	private:
		std::unordered_map<std::string_view, std::function<void(const std::shared_ptr<StreamContext>& streamContext)>> _applyParameters;
	};

	class ComputeMaterial final : public Core::ComputeMaterial, public MaterialImpl<Core::ComputeMaterial>
	{
	public:
		ComputeMaterial(const std::shared_ptr<Core::ComputeShader>& computeShader);
		std::shared_ptr<Core::BaseTask> CreateTaskForApply() override;
		std::shared_ptr<Core::BaseTask> CreateTaskForUpdateParametersInternal() override;
	};

	template<class TCoreMaterial>
	inline MaterialImpl<TCoreMaterial>::MaterialImpl() = default;

	template<class TCoreMaterial>
	inline MaterialImpl<TCoreMaterial>::~MaterialImpl() = default;

	template<class TCoreMaterial>
	inline void MaterialImpl<TCoreMaterial>::OnParametersUpdated(const Core::BaseMaterial::Parameters& params)
	{
		_applyParameters.clear();

		const auto& allParams = params.GetAll();

		for(std::size_t i = 0; i < allParams.size(); ++i)
		{
			const auto& p = allParams.at(i);
			const auto index = static_cast<std::uint32_t>(i);

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
	inline void MaterialImpl<TCoreMaterial>::ApplyParameters(const std::shared_ptr<StreamContext>& streamContext)
	{
		for(const auto& [_, fn] : _applyParameters)
		{
			fn(streamContext);
		}
	}
	template<class TCoreMaterial>
	inline MaterialImpl<TCoreMaterial>::ParametersUpdatedTask::ParametersUpdatedTask(const std::shared_ptr<ParametersUpdatedTaskContext>& context) : Core::TaskWithContext<ParametersUpdatedTaskContext>(context)
	{
	}

	template<class TCoreMaterial>
	inline void MaterialImpl<TCoreMaterial>::ParametersUpdatedTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
	}

	template<class TCoreMaterial>
	inline void MaterialImpl<TCoreMaterial>::ParametersUpdatedTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		if (const auto matImpl = this->_internalTaskContext->materialImplPtr.lock())
		{
			matImpl->OnParametersUpdated(*this->_internalTaskContext->params);
		}
	}

	template<class TCoreMaterial>
	inline void MaterialImpl<TCoreMaterial>::ParametersUpdatedTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}

	template<class TCoreMaterial>
	inline MaterialImpl<TCoreMaterial>::ApplyParametersTask::ApplyParametersTask(const std::shared_ptr<ApplyMaterialTaskContext>& context) : Core::TaskWithContext<ApplyMaterialTaskContext>(context)
	{
	}

	template<class TCoreMaterial>
	inline void MaterialImpl<TCoreMaterial>::ApplyParametersTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);
	}

	template<class TCoreMaterial>
	inline void MaterialImpl<TCoreMaterial>::ApplyParametersTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);
		if(const auto matImpl = this->_internalTaskContext->materialImplPtr.lock() ; const auto sc = _specificStreamContext.lock())
		{
			matImpl->ApplyParameters(sc);
		}
	}

	template<class TCoreMaterial>
	inline void MaterialImpl<TCoreMaterial>::ApplyParametersTask::OnComplete(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnComplete(stream);
	}
}
