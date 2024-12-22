#pragma once
#include <d3d12.h>
#include <Core/Heap.hpp>
#include <wrl/client.h>

namespace MMPEngine::Backend::Dx12
{
	class ResourceEntity;

	class ResourceEntityHeap final : public Core::Heap
	{
	public:
		struct RESettings final
		{
			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
			D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT;
		};
		struct Settings final
		{
			RESettings re;
			Core::Heap::Settings base = {D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, 2};
		};
		ResourceEntityHeap(const Settings& settings, const Microsoft::WRL::ComPtr<ID3D12Device>& device);
	protected:
		std::unique_ptr<Block> InstantiateBlock(std::size_t size) override;
	private:

		class Block : public Core::Heap::Block
		{
		public:
			Block(std::size_t size, const RESettings& resourceSettings, const Microsoft::WRL::ComPtr<ID3D12Device>& device);
			std::shared_ptr<ResourceEntity> GetEntity() const;
		private:
			std::shared_ptr<ResourceEntity> _entity;
		};

	public:

		class Handle : public Core::Heap::Handle
		{
			friend class ResourceEntityHeap;
		public:
			Handle();
			std::size_t GetOffset() const;
			std::shared_ptr<ResourceEntity> GetBlockEntity() const;
		protected:
			Handle(const std::shared_ptr<Heap>& heap, const Entry& entry, const std::shared_ptr<ResourceEntity>& blockEntity);
		private:
			std::shared_ptr<ResourceEntity> _blockEntity;
		};

		Handle Allocate(const Request& request);
		RESettings _re;
		Microsoft::WRL::ComPtr<ID3D12Device> _device;
	};
}
