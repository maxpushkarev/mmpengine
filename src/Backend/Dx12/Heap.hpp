#pragma once
#include <unordered_set>
#include <Core/Heap.hpp>
#include <Core/Task.hpp>

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
			std::shared_ptr<Core::BaseTask> GetInitTask() const;
		private:
			std::shared_ptr<UploadBuffer> _upload;
			std::shared_ptr<Core::BaseTask> _initTask;
		};

		class InitBlocksTaskContext : public Core::TaskContext
		{
		public:
			std::shared_ptr<ConstantBufferHeap> heap;
		};

		class InitBlocksTask final : public Core::BaseTask
		{
		public:
			InitBlocksTask(const std::shared_ptr<InitBlocksTaskContext>& ctx);
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		private:
			std::shared_ptr<InitBlocksTaskContext> _ctx;
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
		const std::shared_ptr<Core::BaseTask>& GetOrCreateTaskToInitializeBlocks() const;
	private:
		std::unordered_set<std::uint64_t> _initializedBlockIds;
		mutable std::shared_ptr<Core::BaseTask> _initBlockTask;
	};
}
