#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <Core/Pool.hpp>

namespace MMPEngine::Backend::Dx12
{
	class BaseDescriptorHeap : public Core::Pool
	{
	protected:
		struct NativeSettings final
		{
			D3D12_DESCRIPTOR_HEAP_TYPE type;
			D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		};
		struct Settings final
		{
			Core::Pool::Settings base;
			NativeSettings native;
		};

		BaseDescriptorHeap(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Settings& settings);
		std::unique_ptr<Core::Pool::Block> InstantiateBlock(std::uint32_t size) override;
		D3D12_CPU_DESCRIPTOR_HANDLE GetNativeCPUDescriptorHandle(const Entry& entry) const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetNativeGPUDescriptorHandle(const Entry& entry) const;

		class Block final : public Core::Pool::Block
		{
		public:
			Block(std::uint32_t size, const Microsoft::WRL::ComPtr<ID3D12Device>& device, const NativeSettings& nativeSettings);
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> native;
		};
		NativeSettings _nativeSettings;
		Microsoft::WRL::ComPtr<ID3D12Device> _device;
		std::uint32_t _incrementSize;
	public:
		class Handle final : public Core::Pool::Handle
		{
			friend class BaseDescriptorHeap;
		public:
			Handle();
			const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUDescriptorHandle() const;
			const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUDescriptorHandle() const;
		protected:
			Handle(const std::shared_ptr<BaseDescriptorHeap>& descHeap, const Entry& entry);
		private:
			std::weak_ptr<BaseDescriptorHeap> _descHeap;
			D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle = { 0 };
			D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle = { 0 };
			D3D12_DESCRIPTOR_HEAP_FLAGS _heapFlags;
		};
		Handle Allocate();
		void CollectNativeBlocks(std::vector<ID3D12DescriptorHeap*>& nativeHeaps) const; 
	private:
		mutable std::optional<std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>>> _nativeBlocksCache;
		void ResetCache();
		void RebuildCacheIfNeed() const;
	};

	template<D3D12_DESCRIPTOR_HEAP_TYPE TDescriptorHeapType>
	class DescriptorPool : public BaseDescriptorHeap
	{
	protected:
		DescriptorPool(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Core::Pool::Settings& baseSettings, D3D12_DESCRIPTOR_HEAP_FLAGS flags);
	};

	template<D3D12_DESCRIPTOR_HEAP_TYPE TDescriptorHeapType>
	inline DescriptorPool<TDescriptorHeapType>::DescriptorPool(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Core::Pool::Settings& baseSettings, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
		: BaseDescriptorHeap(device, {baseSettings, {TDescriptorHeapType, flags}})
	{
	}

	class RTVDescriptorPool final : public DescriptorPool<D3D12_DESCRIPTOR_HEAP_TYPE_RTV>
	{
	public:
		RTVDescriptorPool(
			const Microsoft::WRL::ComPtr<ID3D12Device>& device, 
			const Core::Pool::Settings& baseSettings);
	};

	class DSVDescriptorPool final : public DescriptorPool<D3D12_DESCRIPTOR_HEAP_TYPE_DSV>
	{
	public:
		DSVDescriptorPool(
			const Microsoft::WRL::ComPtr<ID3D12Device>& device,
			const Core::Pool::Settings& baseSettings);
	};

	class CBVSRVUAVDescriptorPool final : public DescriptorPool<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>
	{
	public:
		CBVSRVUAVDescriptorPool(
			const Microsoft::WRL::ComPtr<ID3D12Device>& device,
			const Core::Pool::Settings& baseSettings,
			D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	};
}