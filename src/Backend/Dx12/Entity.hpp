#pragma once
#include <d3d12.h>
#include <Core/Entity.hpp>
#include <Backend/Dx12/Task.hpp>
#include <Backend/Dx12/Pool.hpp>
#include <wrl/client.h>

namespace MMPEngine::Backend::Dx12
{
	class BaseEntity : public virtual std::enable_shared_from_this<Core::BaseEntity>
	{
	public:
		BaseEntity();
		BaseEntity(const BaseEntity&) = delete;
		BaseEntity(BaseEntity&&) noexcept = delete;
		BaseEntity& operator=(const BaseEntity&) = delete;
		BaseEntity& operator=(BaseEntity&&) noexcept = delete;
		virtual ~BaseEntity();

		virtual std::shared_ptr<Core::BaseTask> CreateSwitchStateTask(D3D12_RESOURCE_STATES nextStateMask) = 0;
		virtual Microsoft::WRL::ComPtr<ID3D12Resource> GetNativeResource() const = 0;
		virtual D3D12_GPU_VIRTUAL_ADDRESS GetNativeGPUAddressWithRequiredOffset() const = 0;
		virtual const BaseDescriptorPool::Handle* GetResourceDescriptorHandle() const;
		virtual const BaseDescriptorPool::Handle* GetRTVDescriptorHandle() const;
		virtual const BaseDescriptorPool::Handle* GetDSVDescriptorHandle() const;
	};

	class ResourceEntity : public BaseEntity
	{
	protected:
		ResourceEntity();
	public:
		static constexpr auto kDefaultState = D3D12_RESOURCE_STATE_COMMON;
		class SwitchStateTaskContext final : public Core::EntityTaskContext<ResourceEntity>
		{
		public:
			D3D12_RESOURCE_STATES nextStateMask = D3D12_RESOURCE_STATE_COMMON;
		};
		class SwitchStateTask final : public Task<SwitchStateTaskContext>
		{
		public:
			SwitchStateTask(const std::shared_ptr<SwitchStateTaskContext>& context);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		std::shared_ptr<Core::BaseTask> CreateSwitchStateTask(D3D12_RESOURCE_STATES nextStateMask) override;
		Microsoft::WRL::ComPtr<ID3D12Resource> GetNativeResource() const override;
		D3D12_GPU_VIRTUAL_ADDRESS GetNativeGPUAddressWithRequiredOffset() const override;
	protected:
		void SetNativeResource(const Microsoft::WRL::ComPtr<ID3D12Resource>& nativeResource, std::uint32_t offsetInsideResource);
		Microsoft::WRL::ComPtr<ID3D12Resource> _nativeResource;
		std::uint32_t _offsetInsideResource = 0;
	private:
		D3D12_RESOURCE_STATES _currentStateMask = kDefaultState;
	};
}
