#include <Backend/Vulkan/Entity.hpp>

namespace MMPEngine::Backend::Vulkan
{
	BaseEntity::BaseEntity() = default;
	BaseEntity::~BaseEntity() = default;

	const DeviceMemoryHeap::Handle* BaseEntity::GetDeviceMemoryHeapHandle() const
	{
		return nullptr;
	}

}