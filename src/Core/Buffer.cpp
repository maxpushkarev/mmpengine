#include <Core/Buffer.hpp>

namespace MMPEngine::Core
{
	Buffer::Buffer(const Settings& settings) : _settings(settings)
	{
	}

	std::shared_ptr<Buffer> Buffer::GetUnderlyingBuffer()
	{
		return std::dynamic_pointer_cast<Buffer>(shared_from_this());
	}

	std::shared_ptr<BaseTask> Buffer::CopyToBuffer(const std::shared_ptr<Buffer>& dst)
	{
		return CreateCopyToBufferTask(dst, _settings.byteLength, 0, 0);
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