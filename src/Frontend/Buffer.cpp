#include <Frontend/Buffer.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Buffer.hpp>
#endif

namespace MMPEngine::Frontend
{
	template<>
	Buffer<Core::VertexBuffer, Core::InputAssemblerBuffer::Settings>::Buffer(const std::shared_ptr<Core::AppContext>& appContext, const Core::InputAssemblerBuffer::Settings& settings)
		: Core::InputAssemblerBuffer(settings), Core::VertexBuffer(settings)
	{
		_impl = CreateImpl(appContext);
	}

	template<>
	Buffer<Core::IndexBuffer, Core::InputAssemblerBuffer::Settings>::Buffer(const std::shared_ptr<Core::AppContext>& appContext, const Core::InputAssemblerBuffer::Settings& settings)
		: Core::InputAssemblerBuffer(settings), Core::IndexBuffer(settings)
	{
		_impl = CreateImpl(appContext);
	}

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

	template<>
	std::shared_ptr<Core::VertexBuffer> Buffer<Core::VertexBuffer, Core::InputAssemblerBuffer::Settings>::CreateImpl(const std::shared_ptr<Core::AppContext>& appContext)
	{
		const Core::InputAssemblerBuffer::Settings settings = {this->_ia, this->_settings};

		if (appContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::VertexBuffer>(settings);
#else
			throw Core::UnsupportedException("unable to create vertex buffer for DX12 backend");
#endif
		}

		return nullptr;
	}

	template<>
	std::shared_ptr<Core::IndexBuffer> Buffer<Core::IndexBuffer, Core::InputAssemblerBuffer::Settings>::CreateImpl(const std::shared_ptr<Core::AppContext>& appContext)
	{
		const Core::InputAssemblerBuffer::Settings settings = { this->_ia, this->_settings };

		if (appContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::IndexBuffer>(settings);
#else
			throw Core::UnsupportedException("unable to create index buffer for DX12 backend");
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

	VertexBuffer::VertexBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings) : Core::BaseEntity(settings.base.name), Core::InputAssemblerBuffer(settings), Buffer(appContext, settings)
	{
	}

	IndexBuffer::IndexBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings) : Core::BaseEntity(settings.base.name), Core::InputAssemblerBuffer(settings), Buffer(appContext, settings)
	{
	}
}