#include <Backend/Dx12/Heap.hpp>
#include <Backend/Dx12/d3dx12.h>
#include <Backend/Dx12/Entity.hpp>
#include <Backend/Dx12/Buffer.hpp>

namespace MMPEngine::Backend::Dx12
{
	ConstantBufferHeap::ConstantBufferHeap(const Settings& settings)
		: Core::Heap(settings)
	{
	}

	std::unique_ptr<Core::Heap::Block> ConstantBufferHeap::InstantiateBlock(std::size_t size)
	{
		return std::make_unique<Block>(size);
	}

	ConstantBufferHeap::Block::Block(std::size_t size) : Core::Heap::Block(size)
	{
		_upload = std::make_shared<UploadBuffer>(Core::Buffer::Settings {
			size, "constant_buffer_block_in_heap"
		});
		_initTask = _upload->CreateInitializationTask();
	}

	std::shared_ptr<UploadBuffer> ConstantBufferHeap::Block::GetEntity() const
	{
		return _upload;
	}

	std::shared_ptr<Core::BaseTask> ConstantBufferHeap::Block::GetInitTask() const
	{
		return _initTask;
	}

	ConstantBufferHeap::Handle::Handle(const std::shared_ptr<Heap>& heap, const Entry& entry, const std::shared_ptr<UploadBuffer>& blockEntity)
		: Core::Heap::Handle(heap, entry), _uploadBlock(blockEntity)
	{
	}

	ConstantBufferHeap::Handle::Handle() = default;

	std::shared_ptr<UploadBuffer> ConstantBufferHeap::Handle::GetUploadBlock() const
	{
		return _uploadBlock;
	}

	std::size_t ConstantBufferHeap::Handle::GetOffset() const
	{
		return _entry.value().range.from;
	}

	ConstantBufferHeap::Handle ConstantBufferHeap::Allocate(const Request& request)
	{
		const auto entry = AllocateEntry(request);
		const auto block = dynamic_cast<Block*>(_blocks[entry.blockIndex].get());
		return { shared_from_this(), entry, block->GetEntity()};
	}

	const std::shared_ptr<Core::BaseTask>& ConstantBufferHeap::GetOrCreateTaskToInitializeBlocks() const
	{
		if(!_initBlockTask)
		{
			const auto ctx = std::make_shared<InitBlocksTaskContext>();
			ctx->heap = std::dynamic_pointer_cast<ConstantBufferHeap>(std::const_pointer_cast<Core::Heap>(shared_from_this()));
			_initBlockTask = std::make_shared<InitBlocksTask>(ctx);		
		}

		return _initBlockTask;
	}


	ConstantBufferHeap::InitBlocksTask::InitBlocksTask(const std::shared_ptr<InitBlocksTaskContext>& ctx)
		: _ctx(ctx)
	{
	}

	void ConstantBufferHeap::InitBlocksTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Core::BaseTask::OnScheduled(stream);

		const auto heap = _ctx->heap;

		for(const auto& b : heap->_blocks)
		{
			const auto castedBlock = dynamic_cast<Block*>(b.get());
			const auto id = castedBlock->GetEntity()->GetId();

			if(heap->_initializedBlockIds.find(id) == heap->_initializedBlockIds.cend())
			{
				heap->_initializedBlockIds.emplace(id);
				stream->Schedule(castedBlock->GetInitTask());
			}
		}
	}

}
