#pragma once
#include <Core/Camera.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Camera.hpp>
#endif

namespace MMPEngine::Frontend
{
	template<typename TCoreCamera>
	class Camera : public TCoreCamera
	{
	protected:
		Camera(const std::shared_ptr<Core::GlobalContext>& globalContext, const typename TCoreCamera::Settings& settings, const std::shared_ptr<Core::Node>& node);
		void FillData(const std::shared_ptr<Core::GlobalContext>& globalContext, Core::Camera::Data& data) override;
	public:
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::ContextualTask<Core::Camera::UpdateDataTaskContext>> CreateTaskToUpdateUniformData() override;
		std::shared_ptr<Core::BaseEntity> GetUniformDataEntity() const override;
		std::shared_ptr<const Core::Node> GetNode() const override;
	protected:
		std::shared_ptr<TCoreCamera> _impl;

	};

	class PerspectiveCamera final : public Camera<Core::PerspectiveCamera>
	{
	public:
		PerspectiveCamera(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings, const std::shared_ptr<Core::Node>& node);
	};

	class OrthographicCamera final : public Camera<Core::OrthographicCamera>
	{
	public:
		OrthographicCamera(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings, const std::shared_ptr<Core::Node>& node);
	};

	template<typename TCoreCamera>
	Camera<TCoreCamera>::Camera(const std::shared_ptr<Core::GlobalContext>& globalContext, const typename TCoreCamera::Settings& settings, const std::shared_ptr<Core::Node>& node)
		: TCoreCamera(settings, node)
	{
		if constexpr (std::is_same_v<TCoreCamera, Core::PerspectiveCamera>)
		{
			if (globalContext->settings.backend == Core::BackendType::Dx12)
			{
#ifdef MMPENGINE_BACKEND_DX12
				_impl = std::make_shared<Backend::Dx12::PerspectiveCamera>(settings, node);
#else
				throw Core::UnsupportedException("unable to create perspective camera for DX12 backend");
#endif
			}
		}

		if constexpr (std::is_same_v<TCoreCamera, Core::OrthographicCamera>)
		{
			if (globalContext->settings.backend == Core::BackendType::Dx12)
			{
#ifdef MMPENGINE_BACKEND_DX12
				_impl = std::make_shared<Backend::Dx12::OrthographicCamera>(settings, node);
#else
				throw Core::UnsupportedException("unable to create orthographic camera for DX12 backend");
#endif
			}
		}
	}

	template <typename TCoreCamera>
	void Camera<TCoreCamera>::FillData(const std::shared_ptr<Core::GlobalContext>& globalContext, Core::Camera::Data& data)
	{
		throw std::logic_error{"Impossible"};
	}

	template <typename TCoreCamera>
	std::shared_ptr<Core::BaseTask> Camera<TCoreCamera>::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();
	}

	template <typename TCoreCamera>
	std::shared_ptr<const Core::Node> Camera<TCoreCamera>::GetNode() const
	{
		return _impl->GetNode();
	}

	template <typename TCoreCamera>
	std::shared_ptr<Core::BaseEntity> Camera<TCoreCamera>::GetUniformDataEntity() const
	{
		return _impl->GetUniformDataEntity();
	}

	template <typename TCoreCamera>
	std::shared_ptr<Core::ContextualTask<Core::Camera::UpdateDataTaskContext>> Camera<TCoreCamera>::CreateTaskToUpdateUniformData()
	{
		return _impl->CreateTaskToUpdateUniformData();
	}
}