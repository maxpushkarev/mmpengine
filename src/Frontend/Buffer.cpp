#include <Frontend/Buffer.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Buffer.hpp>
#endif

namespace MMPEngine::Frontend
{
	template<>
	std::shared_ptr<Core::UploadBuffer> Buffer<Core::UploadBuffer>::CreateImpl(const std::shared_ptr<Core::AppContext>& appContext)
	{
		if (appContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::UploadBuffer>(this->_settings);
#else
			throw Core::UnsupportedException("unable to create upload buffer for DX12 backend");
#endif
		}

		return nullptr;
	}

	template<>
	std::shared_ptr<Core::ResidentBuffer> Buffer<Core::ResidentBuffer>::CreateImpl(const std::shared_ptr<Core::AppContext>& appContext)
	{
		if (appContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::ResidentBuffer>(this->_settings);
#else
			throw Core::UnsupportedException("unable to create resident buffer for DX12 backend");
#endif
		}

		return nullptr;
	}

	template<>
	std::shared_ptr<Core::ReadBackBuffer> Buffer<Core::ReadBackBuffer>::CreateImpl(const std::shared_ptr<Core::AppContext>& appContext)
	{
		if (appContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::ReadBackBuffer>(this->_settings);
#else
			throw Core::UnsupportedException("unable to create readback buffer for DX12 backend");
#endif
		}

		return nullptr;
	}

	UploadBuffer::UploadBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings) : Core::BaseEntity(settings.name), Buffer(appContext, settings)
	{
	}

	void UploadBuffer::Write(const void* src, std::size_t byteLength, std::size_t byteOffset)
	{
		_impl->Write(src, byteLength, byteOffset);
	}

	ReadBackBuffer::ReadBackBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings) : Core::BaseEntity(settings.name), Buffer(appContext, settings)
	{
	}

	void ReadBackBuffer::Read(void* dst, std::size_t byteLength, std::size_t byteOffset)
	{
		_impl->Read(dst, byteLength, byteOffset);
	}

	ResidentBuffer::ResidentBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings) : Core::BaseEntity(settings.name), Buffer(appContext, settings)
	{
	}
}