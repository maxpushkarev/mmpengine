#pragma once
#include <d3d12.h>
#include <Core/Heap.hpp>
#include <wrl/client.h>

namespace MMPEngine::Backend::Dx12
{
	class UploadBuffer;

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
			std::shared_ptr<UploadBuffer> GetEntity() const;
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
