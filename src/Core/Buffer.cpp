#include <Core/Buffer.hpp>

namespace MMPEngine::Core
{
	Buffer::Buffer(const Settings& settings) : BaseEntity(settings.name), _settings(settings)
	{
		std::string empty {};
		_settings.name.swap(empty);
	}

	UploadBuffer::UploadBuffer(const Settings& settings) : Buffer(settings)
	{
	}

	ReadBackBuffer::ReadBackBuffer(const Settings& settings) : Buffer(settings)
	{
	}

	InputAssemblerBuffer::InputAssemblerBuffer(const Settings& settings) : Buffer(settings.base), _ia(settings.ia)
	{
	}

	ResidentBuffer::ResidentBuffer(const Settings& settings) : Buffer(settings)
	{
	}
}