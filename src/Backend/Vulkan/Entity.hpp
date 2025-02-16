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

		virtual const DeviceMemoryHeap::Handle* GetDeviceMemoryHeapHandle() const;
	};
}