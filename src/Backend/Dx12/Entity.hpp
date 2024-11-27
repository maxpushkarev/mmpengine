#pragma once
#include <d3d12.h>
#include <Core/Entity.hpp>
#include <Backend/Dx12/Task.hpp>
#include <wrl/client.h>

namespace MMPEngine::Backend::Dx12
{
	class BaseEntity : Core::BaseEntity
	{
	protected:
		BaseEntity(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource, std::string_view name);
		BaseEntity(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource);
	private:
		void SetUpNativeResourceName();
	protected:
		class SwitchStateTaskContext final : public Core::TaskContext
		{
		public:
			SwitchStateTaskContext(const std::shared_ptr<BaseEntity>& entity, D3D12_RESOURCE_STATES nextState);
			D3D12_RESOURCE_STATES nextStateMask;
			std::weak_ptr<BaseEntity> entity;
		};
		class SwitchStateTask final : public Task, public Core::TaskWithInnerContext<SwitchStateTaskContext>
		{
		public:
			SwitchStateTask(const std::shared_ptr<SwitchStateTaskContext>& context);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			void Finalize(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	protected:
		static constexpr auto _defaultState = D3D12_RESOURCE_STATE_COMMON;
		Microsoft::WRL::ComPtr<ID3D12Resource> _nativeResource;
		D3D12_RESOURCE_STATES _currentStateMask = _defaultState;
	};
}
