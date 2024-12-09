#pragma once
#include <d3d12.h>
#include <Core/Entity.hpp>
#include <Backend/Dx12/Task.hpp>
#include <wrl/client.h>

namespace MMPEngine::Backend::Dx12
{
	class BaseEntity : public virtual Core::BaseEntity
	{
	public:
		virtual std::shared_ptr<Core::BaseTask> CreateSwitchStateTask(D3D12_RESOURCE_STATES nextStateMask) = 0;
		virtual Microsoft::WRL::ComPtr<ID3D12Resource> GetNativeResource() const = 0;
	};

	class ResourceEntity : public BaseEntity
	{
	protected:
		ResourceEntity(std::string_view name);
		ResourceEntity();
	public:

		class SwitchStateTaskContext final : public EntityTaskContext<ResourceEntity>
		{
		public:
			D3D12_RESOURCE_STATES nextStateMask;
		};
		class SwitchStateTask final : public Task, public Core::TaskWithInternalContext<SwitchStateTaskContext>
		{
		public:
			SwitchStateTask(const std::shared_ptr<SwitchStateTaskContext>& context);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			void OnComplete(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		virtual std::shared_ptr<Core::BaseTask> CreateSwitchStateTask(D3D12_RESOURCE_STATES nextStateMask) override;
	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> GetNativeResource() const override;
		void SetNativeResource(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource);
		Microsoft::WRL::ComPtr<ID3D12Resource> _nativeResource;
		static constexpr auto _defaultState = D3D12_RESOURCE_STATE_COMMON;
	private:
		void SetUpNativeResourceName();
	private:
		D3D12_RESOURCE_STATES _currentStateMask = _defaultState;
	};
}
