#pragma once
#include <memory>
#include <optional>

namespace MMPEngine::Core
{
	class BaseHeap : public std::enable_shared_from_this<BaseHeap>
	{
	protected:
		struct Entry final
		{
			std::uint32_t heapIndex;
			std::uint32_t slotIndexInHeap;
		};
	public:
		struct Settings
		{
			std::uint32_t initialCapacity = 4;
			std::uint32_t growthFactor = 2;
		};
		class Handle
		{
		protected:
			Handle(const std::shared_ptr<BaseHeap>& heap, const Entry& entry);
		public:
			Handle(const Handle&) = delete;
			Handle(Handle&& movableHandle) noexcept;
			Handle& operator=(const Handle&) = delete;
			Handle& operator=(Handle&& movableHandle) noexcept;
			virtual ~Handle();
		protected:
			std::optional<Entry> _entry;
			std::weak_ptr<BaseHeap> _heap;
		};
	protected:
		BaseHeap(const Settings& settings);
		virtual ~BaseHeap();
		virtual void Release(const Entry& entry) = 0;

	public:
		BaseHeap(const BaseHeap&) = delete;
		BaseHeap(BaseHeap&&) noexcept = delete;
		BaseHeap& operator=(const BaseHeap&) = delete;
		BaseHeap& operator=(BaseHeap&&) noexcept = delete;
	protected:
		Settings _settings;
	};
}