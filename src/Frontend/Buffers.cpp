#include <Frontend/Buffers.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Buffer.hpp>
#endif

namespace MMPEngine::Frontend
{
	UploadBuffer::UploadBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings) : Core::BaseEntity(settings.name), Core::UploadBuffer(settings)
	{
		if (appContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::UploadBuffer>(settings);
#else
			throw Core::UnsupportedException("unable to create upload buffer for DX12 backend");
#endif
		}
	}

	std::shared_ptr<Core::BaseTask> UploadBuffer::CreateCopyToBufferTask(const std::shared_ptr<Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset)
	{
		return _impl->CreateCopyToBufferTask(dst->GetUnderlyingBuffer(), byteLength, srcByteOffset, dstByteOffset);
	}

	std::shared_ptr<Core::BaseTask> UploadBuffer::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();
	}

	void UploadBuffer::Write(const void* src, std::size_t byteLength, std::size_t byteOffset)
	{
		_impl->Write(src, byteLength, byteOffset);
	}

	std::shared_ptr<Core::Buffer> UploadBuffer::GetUnderlyingBuffer()
	{
		return _impl->GetUnderlyingBuffer();
	}

	ReadBackBuffer::ReadBackBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings) : Core::BaseEntity(settings.name), Core::ReadBackBuffer(settings)
	{
		if (appContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::ReadBackBuffer>(settings);
#else
			throw Core::UnsupportedException("unable to create readback buffer for DX12 backend");
#endif
		}
	}

	std::shared_ptr<Core::BaseTask> ReadBackBuffer::CreateCopyToBufferTask(const std::shared_ptr<Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset)
	{
		return _impl->CreateCopyToBufferTask(dst->GetUnderlyingBuffer(), byteLength, srcByteOffset, dstByteOffset);
	}

	std::shared_ptr<Core::BaseTask> ReadBackBuffer::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();
	}

	void ReadBackBuffer::Read(void* dst, std::size_t byteLength, std::size_t byteOffset)
	{
		_impl->Read(dst, byteLength, byteOffset);
	}

	std::shared_ptr<Core::Buffer> ReadBackBuffer::GetUnderlyingBuffer()
	{
		return _impl->GetUnderlyingBuffer();
	}
}