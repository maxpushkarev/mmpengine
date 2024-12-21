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

		ASSERT_EQ(h2->GetLength(), 4);
		ASSERT_EQ(h2->GetOffset(), 2);

		ASSERT_EQ(h3->GetLength(), 8);
		ASSERT_EQ(h3->GetOffset(), 6);

		h2.reset();
		h2 = std::make_unique<Heap::Handle>(_heap->Allocate({ 3 }));

		ASSERT_EQ(h2->GetLength(), 3);
		ASSERT_EQ(h2->GetOffset(), 2);

		const auto h4 = std::make_unique<Heap::Handle>(_heap->Allocate({ 1 }));
		ASSERT_EQ(h4->GetLength(), 1);
		ASSERT_EQ(h4->GetOffset(), 5);
	};
}