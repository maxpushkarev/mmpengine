#pragma once
#include <Core/Entity.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class DeviceMemoryBlock final : public Core::BaseEntity
	{
	public:
		struct Settings final
		{
			std::size_t byteSize;
		};

		DeviceMemoryBlock(const Settings&);
	private:
		Settings _settings;
	};
}