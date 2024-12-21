#include <gtest/gtest.h>
#include <Core/Heap.hpp>

namespace MMPEngine::Core::Tests
{
	class Heap final : public Core::Heap
	{
	public:
		class Handle final : public Core::Heap::Handle
		{
		public:
			Handle(const std::shared_ptr<Core::Heap>& heap, const Entry& entry) : Core::Heap::Handle(heap, entry)
			{
			}
			std::size_t GetOffset() const
			{
				return _entry.value().range.from;
			}
			std::size_t GetLength() const
			{
				return _entry.value().range.GetLength();
			}
			std::size_t GetBlockIndex() const
			{
				return _entry.value().blockIndex;
			}
		};
		Handle Allocate(const Core::Heap::Request& request)
		{
			const auto entry = AllocateEntry(request);
			return {shared_from_this(), entry};
		}
		Heap(const Core::Heap::Settings& settings) : Core::Heap(settings)
		{
		}
	};

	class HeapTests : public testing::Test
	{
	protected:
		std::shared_ptr<Heap> _heap;

		inline void SetUp() override
		{
			testing::Test::SetUp();
			_heap = std::make_shared<Heap>(Core::Heap::Settings{1024, 2});
		}

		inline void TearDown() override
		{
			_heap.reset();
			testing::Test::TearDown();
		}
	};


	TEST_F(HeapTests, SmallAllocations)
	{
		const auto h1 = std::make_unique<Heap::Handle>(_heap->Allocate({2 }));
		auto h2 = std::make_unique<Heap::Handle>(_heap->Allocate({ 4 }));
		const auto h3 = std::make_unique<Heap::Handle>(_heap->Allocate({ 8 }));

		ASSERT_EQ(h1->GetLength(), 2);
		ASSERT_EQ(h1->GetOffset(), 0);
		ASSERT_EQ(h1->GetBlockIndex(), 0);

		ASSERT_EQ(h2->GetLength(), 4);
		ASSERT_EQ(h2->GetOffset(), 2);
		ASSERT_EQ(h2->GetBlockIndex(), 0);

		ASSERT_EQ(h3->GetLength(), 8);
		ASSERT_EQ(h3->GetOffset(), 6);
		ASSERT_EQ(h3->GetBlockIndex(), 0);

		h2.reset();
		h2 = std::make_unique<Heap::Handle>(_heap->Allocate({ 3 }));

		ASSERT_EQ(h2->GetLength(), 3);
		ASSERT_EQ(h2->GetOffset(), 2);
		ASSERT_EQ(h2->GetBlockIndex(), 0);

		const auto h4 = std::make_unique<Heap::Handle>(_heap->Allocate({ 1 }));
		ASSERT_EQ(h4->GetLength(), 1);
		ASSERT_EQ(h4->GetOffset(), 5);
		ASSERT_EQ(h4->GetBlockIndex(), 0);
	};

	TEST_F(HeapTests, BigAllocation)
	{
		const auto h1 = std::make_unique<Heap::Handle>(_heap->Allocate({ 2048 }));
		auto h2 = std::make_unique<Heap::Handle>(_heap->Allocate({ 4096 }));

		ASSERT_EQ(0, h1->GetBlockIndex());
		ASSERT_EQ(1, h2->GetBlockIndex());

		ASSERT_EQ(0, h1->GetOffset());
		ASSERT_EQ(0, h2->GetOffset());

		h2.reset();
		const auto h3 = std::make_unique<Heap::Handle>(_heap->Allocate({ 32 }));
		ASSERT_EQ(1, h3->GetBlockIndex());
		ASSERT_EQ(0, h3->GetOffset());
	}

	TEST_F(HeapTests, NoDefragmentation)
	{
		const auto h1 = std::make_unique<Heap::Handle>(_heap->Allocate({ 3 }));
		auto h2 = std::make_unique<Heap::Handle>(_heap->Allocate({ 5 }));
		auto h3 = std::make_unique<Heap::Handle>(_heap->Allocate({ 3 }));
		const auto h4 = std::make_unique<Heap::Handle>(_heap->Allocate({ 2 }));

		h2.reset();
		h3.reset();

		const auto h5 = std::make_unique<Heap::Handle>(_heap->Allocate({ 7 }));
		ASSERT_EQ(h5->GetLength(), 7);
		ASSERT_EQ(h5->GetOffset(), 3);
	}

	TEST_F(HeapTests, Alignment)
	{
		const auto h1 = std::make_unique<Heap::Handle>(_heap->Allocate({ 2, 4 }));
		const auto h2 = std::make_unique<Heap::Handle>(_heap->Allocate({ 6, 4 }));
		auto h3 = std::make_unique<Heap::Handle>(_heap->Allocate({ 1 }));
		auto h4 = std::make_unique<Heap::Handle>(_heap->Allocate({ 1 }));

		ASSERT_EQ(h1->GetLength(), 2);
		ASSERT_EQ(h1->GetOffset(), 0);

		ASSERT_EQ(h2->GetLength(), 6);
		ASSERT_EQ(h2->GetOffset(), 4);

		ASSERT_EQ(h3->GetLength(), 1);
		ASSERT_EQ(h3->GetOffset(), 2);

		ASSERT_EQ(h4->GetLength(), 1);
		ASSERT_EQ(h4->GetOffset(), 3);

		h3.reset();
		h4.reset();

		const auto h5 = std::make_unique<Heap::Handle>(_heap->Allocate({ 3, 2 }));
		ASSERT_EQ(h5->GetLength(), 3);
		ASSERT_EQ(h5->GetOffset(), 10);

		const auto h6 = std::make_unique<Heap::Handle>(_heap->Allocate({ 2, 2 }));
		ASSERT_EQ(h6->GetLength(), 2);
		ASSERT_EQ(h6->GetOffset(), 2);
	}
}