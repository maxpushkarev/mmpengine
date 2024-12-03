#include <Core/Buffer.hpp>
#include <cmath>

namespace MMPEngine::Core
{
	const Buffer::Settings& Buffer::GetSettingsInternal()
	{
		return _settings;
	}

	Buffer::Buffer(const Settings& settings) : _settings(settings)
	{
	}

	std::shared_ptr<Buffer> Buffer::GetUnderlyingBuffer() const
	{
		const auto underlyingBuffer = const_cast<Buffer*>(this)->GetUnderlyingBuffer();
		return underlyingBuffer;
	}

	std::shared_ptr<Buffer> Buffer::GetUnderlyingBuffer()
	{
		return std::dynamic_pointer_cast<Buffer>(shared_from_this());
	}

	const Buffer::Settings& Buffer::GetSettings() const
	{
		return GetUnderlyingBuffer()->GetSettingsInternal();
	}

	std::shared_ptr<BaseTask> Buffer::CopyToBuffer(const std::shared_ptr<Buffer>& dst) const
	{
		const auto dstUnderlyingBuffer = dst->GetUnderlyingBuffer();
		const auto minMax = std::minmax(GetSettings().byteLength, dstUnderlyingBuffer->GetSettings().byteLength);
		return GetUnderlyingBuffer()->CreateCopyToBufferTask(dstUnderlyingBuffer, minMax.first, 0, 0);
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