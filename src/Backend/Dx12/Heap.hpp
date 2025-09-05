#pragma once
#include <Core/Heap.hpp>
#include <Core/Task.hpp>
#include <Backend/Dx12/Memory.hpp>

namespace MMPEngine::Backend::Dx12
{
	class UploadBuffer;


	class DeviceMemoryHeap final : public Core::Heap
	{
	private:
		class Block final : public Core::Heap::Block
		{
		public:
			Block(const D3D12_HEAP_DESC& heapDesc);
			std::shared_ptr<Core::BaseEntity> GetEntity() const override;
			std::shared_ptr<DeviceMemoryBlock> GetEntityBlock() const;
		private:
			std::shared_ptr<DeviceMemoryBlock> _entity;
		};

	public:
		class Handle final : public Core::Heap::Handle
		{
			friend DeviceMemoryHeap;
		public:
			Handle();
			std::size_t GetOffset() const;
			std::size_t GetSize() const;
			std::shared_ptr<DeviceMemoryBlock> GetMemoryBlock() const;
		protected:
			Handle(const std::shared_ptr<Heap>& heap, const Entry& entry, const std::shared_ptr<DeviceMemoryBlock>& deviceMemoryBlock);
		private:
			std::shared_ptr<DeviceMemoryBlock> _deviceMemoryBlock;
		};

		DeviceMemoryHeap(const Settings& settings, const D3D12_HEAP_DESC& desc);
		Handle Allocate(const Request& request);
	protected:
		std::unique_ptr<Heap::Block> InstantiateBlock(std::size_t size) override;
	private:
		const D3D12_HEAP_DESC _desc;
	};

	class ConstantBufferHeap final : public Core::Heap
	{
	public:
		ConstantBufferHeap(const Settings& settings);
	protected:
		std::unique_ptr<Block> InstantiateBlock(std::size_t size) override;
	private:

		class Block : public Core::Heap::Block
		{
		public:
			Block(std::size_t size);
			std::shared_ptr<UploadBuffer> GetUploadBuffer() const;
			std::shared_ptr<Core::BaseEntity> GetEntity() const override;
		private:
			std::shared_ptr<UploadBuffer> _upload;
		};

	public:

		class Handle : public Core::Heap::Handle
		{
			friend class ConstantBufferHeap;
		public:
			Handle();
			std::size_t GetOffset() const;
			std::shared_ptr<UploadBuffer> GetUploadBlock() const;
		protected:
			Handle(const std::shared_ptr<Heap>& heap, const Entry& entry, const std::shared_ptr<UploadBuffer>& blockEntity);
		private:
			std::shared_ptr<UploadBuffer> _uploadBlock;
		};

		Handle Allocate(const Request& request);
	};
}
