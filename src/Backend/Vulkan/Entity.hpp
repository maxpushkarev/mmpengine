#pragma once
#include <Core/Entity.hpp>
#include <Backend/Vulkan/Heap.hpp>

namespace MMPEngine::Backend::Vulkan
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
	};

	class ResourceEntity : public BaseEntity
	{
	protected:
		virtual std::shared_ptr<DeviceMemoryHeap> GetMemoryHeap(const std::shared_ptr<GlobalContext>& globalContext) const = 0;
	protected:
		DeviceMemoryHeap::Handle _deviceMemoryHeapHandle;
	};
}