#pragma once
#include <d3d12.h>
#include <Core/Entity.hpp>
#include <Backend/Dx12/Task.hpp>
#include <wrl/client.h>

namespace MMPEngine::Backend::Dx12
{
	class BaseEntity : public virtual Core::BaseEntity, public std::enable_shared_from_this<BaseEntity>
	{
	protected:
		BaseEntity(std::string_view name);
		BaseEntity();
	protected:
		class SwitchStateTaskContext final : public Core::TaskContext
		{
		public:
			SwitchStateTaskContext(const std::shared_ptr<BaseEntity>& entity, D3D12_RESOURCE_STATES nextState);
			D3D12_RESOURCE_STATES nextStateMask;
			std::weak_ptr<BaseEntity> entity;
		};
		class SwitchStateTask final : public Task, public Core::TaskWithInternalContext<SwitchStateTaskContext>
		{
		public:
			SwitchStateTask(const std::shared_ptr<SwitchStateTaskContext>& context);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			void Finalize(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	public:
		std::shared_ptr<Core::BaseTask> CreateSwitchStateTask(D3D12_RESOURCE_STATES nextStateMask);
	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> GetNativeResource();
		void SetNativeResource(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource);
	private:
		void SetUpNativeResourceName();
	private:
		static constexpr auto _defaultState = D3D12_RESOURCE_STATE_COMMON;
		Microsoft::WRL::ComPtr<ID3D12Resource> _nativeResource;
		D3D12_RESOURCE_STATES _currentStateMask = _defaultState;
	};
}
