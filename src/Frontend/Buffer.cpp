#include <Frontend/Buffer.hpp>



namespace MMPEngine::Frontend
{
	template<>
	Buffer<Core::VertexBuffer, Core::InputAssemblerBuffer::Settings>::Buffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Core::InputAssemblerBuffer::Settings& settings)
		: Core::VertexBuffer(settings)
	{
		_impl = CreateImpl(globalContext);
	}

	template<>
	Buffer<Core::IndexBuffer, Core::InputAssemblerBuffer::Settings>::Buffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Core::InputAssemblerBuffer::Settings& settings)
		: Core::IndexBuffer(settings)
	{
		_impl = CreateImpl(globalContext);
	}

	template<>
	std::shared_ptr<Core::UploadBuffer> Buffer<Core::UploadBuffer>::CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext)
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::UploadBuffer>(this->_settings);
#else
			throw Core::UnsupportedException("unable to create upload buffer for DX12 backend");
#endif
		}

		if (globalContext->settings.backend == Core::BackendType::Vulkan)
		{
#ifdef MMPENGINE_BACKEND_VULKAN
			return std::make_shared<Backend::Vulkan::UploadBuffer>(this->_settings);
#else
			throw Core::UnsupportedException("unable to create upload buffer for Vulkan backend");
#endif
		}
        
        if (globalContext->settings.backend == Core::BackendType::Metal)
        {
#ifdef MMPENGINE_BACKEND_METAL
            return std::make_shared<Backend::Metal::UploadBuffer>(this->_settings);
#else
            throw Core::UnsupportedException("unable to create upload buffer for Metal backend");
#endif
        }

		return nullptr;
	}

	template<>
	std::shared_ptr<Core::ResidentBuffer> Buffer<Core::ResidentBuffer>::CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext)
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::ResidentBuffer>(this->_settings);
#else
			throw Core::UnsupportedException("unable to create resident buffer for DX12 backend");
#endif
		}

		if (globalContext->settings.backend == Core::BackendType::Vulkan)
		{
#ifdef MMPENGINE_BACKEND_VULKAN
			return std::make_shared<Backend::Vulkan::ResidentBuffer>(this->_settings);
#else
			throw Core::UnsupportedException("unable to create resident buffer for Vulkan backend");
#endif
		}

        if (globalContext->settings.backend == Core::BackendType::Metal)
        {
#ifdef MMPENGINE_BACKEND_METAL
            return std::make_shared<Backend::Metal::ResidentBuffer>(this->_settings);
#else
            throw Core::UnsupportedException("unable to create resident buffer for Metal backend");
#endif
        }
        
		return nullptr;
	}

	template<>
	std::shared_ptr<Core::UnorderedAccessBuffer> Buffer<Core::UnorderedAccessBuffer, Core::BaseUnorderedAccessBuffer::Settings>::CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext)
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::UnorderedAccessBuffer>(this->_uaSettings);
#else
			throw Core::UnsupportedException("unable to create unordered access buffer for DX12 backend");
#endif
		}

		if (globalContext->settings.backend == Core::BackendType::Vulkan)
		{
#ifdef MMPENGINE_BACKEND_VULKAN
			return std::make_shared<Backend::Vulkan::UnorderedAccessBuffer>(this->_uaSettings);
#else
			throw Core::UnsupportedException("unable to create unordered access buffer for Vulkan backend");
#endif
		}

		return nullptr;
	}

	template<>
	std::shared_ptr<Core::CounteredUnorderedAccessBuffer> Buffer<Core::CounteredUnorderedAccessBuffer, Core::BaseUnorderedAccessBuffer::Settings>::CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext)
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::CounteredUnorderedAccessBuffer>(this->_uaSettings);
#else
			throw Core::UnsupportedException("unable to create countered unordered access buffer for DX12 backend");
#endif
		}

		if (globalContext->settings.backend == Core::BackendType::Vulkan)
		{
#ifdef MMPENGINE_BACKEND_VULKAN
			return std::make_shared<Backend::Vulkan::CounteredUnorderedAccessBuffer>(this->_uaSettings);
#else
			throw Core::UnsupportedException("unable to create countered unordered access buffer for Vulkan backend");
#endif
		}

		return nullptr;
	}

	template<>
	std::shared_ptr<Core::ReadBackBuffer> Buffer<Core::ReadBackBuffer>::CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext)
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::ReadBackBuffer>(this->_settings);
#else
			throw Core::UnsupportedException("unable to create readback buffer for DX12 backend");
#endif
		}

		if (globalContext->settings.backend == Core::BackendType::Vulkan)
		{
#ifdef MMPENGINE_BACKEND_VULKAN
			return std::make_shared<Backend::Vulkan::ReadBackBuffer>(this->_settings);
#else
			throw Core::UnsupportedException("unable to create readback buffer for Vulkan backend");
#endif
		}
        
        if (globalContext->settings.backend == Core::BackendType::Metal)
        {
#ifdef MMPENGINE_BACKEND_METAL
            return std::make_shared<Backend::Metal::ReadBackBuffer>(this->_settings);
#else
            throw Core::UnsupportedException("unable to create readback buffer for Metal backend");
#endif
        }

		return nullptr;
	}

	template<>
	std::shared_ptr<Core::VertexBuffer> Buffer<Core::VertexBuffer, Core::InputAssemblerBuffer::Settings>::CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext)
	{
		const Core::InputAssemblerBuffer::Settings settings = {this->_ia, this->_settings};

		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::VertexBuffer>(settings);
#else
			throw Core::UnsupportedException("unable to create vertex buffer for DX12 backend");
#endif
		}

		if (globalContext->settings.backend == Core::BackendType::Vulkan)
		{
#ifdef MMPENGINE_BACKEND_VULKAN
			return std::make_shared<Backend::Vulkan::VertexBuffer>(settings);
#else
			throw Core::UnsupportedException("unable to create vertex buffer for Vulkan backend");
#endif
		}

		return nullptr;
	}

	template<>
	std::shared_ptr<Core::IndexBuffer> Buffer<Core::IndexBuffer, Core::InputAssemblerBuffer::Settings>::CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext)
	{
		const Core::InputAssemblerBuffer::Settings settings = { this->_ia, this->_settings };

		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			return std::make_shared<Backend::Dx12::IndexBuffer>(settings);
#else
			throw Core::UnsupportedException("unable to create index buffer for DX12 backend");
#endif
		}

		if (globalContext->settings.backend == Core::BackendType::Vulkan)
		{
#ifdef MMPENGINE_BACKEND_VULKAN
			return std::make_shared<Backend::Vulkan::IndexBuffer>(settings);
#else
			throw Core::UnsupportedException("unable to create index buffer for Vulkan backend");
#endif
		}


		return nullptr;
	}

	UploadBuffer::UploadBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings) : Buffer(globalContext, settings)
	{
	}

	std::shared_ptr<Core::ContextualTask<UploadBuffer::WriteTaskContext>> UploadBuffer::CreateWriteTask(const void* src, std::size_t byteLength, std::size_t byteOffset)
	{
		return _impl->CreateWriteTask(src, byteLength, byteOffset);
	}

	ReadBackBuffer::ReadBackBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings) : Buffer(globalContext, settings)
	{
	}

	std::shared_ptr<Core::ContextualTask<ReadBackBuffer::ReadTaskContext>> ReadBackBuffer::CreateReadTask(void* dst, std::size_t byteLength, std::size_t byteOffset)
	{
		return _impl->CreateReadTask(dst, byteLength, byteOffset);
	}

	ResidentBuffer::ResidentBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings) : Buffer(globalContext, settings)
	{
	}

	UnorderedAccessBuffer::UnorderedAccessBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings) : Buffer(globalContext, settings)
	{
	}

	CounteredUnorderedAccessBuffer::CounteredUnorderedAccessBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings) : Buffer(globalContext, settings)
	{
	}

	std::shared_ptr<Core::BaseTask> CounteredUnorderedAccessBuffer::CreateResetCounterTask()
	{
		return _impl->CreateResetCounterTask();
	}

	std::shared_ptr<Core::BaseTask> CounteredUnorderedAccessBuffer::CreateCopyCounterTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t dstByteOffset)
	{
		return _impl->CreateCopyCounterTask(dst, dstByteOffset);
	}

	VertexBuffer::VertexBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings) : Buffer(globalContext, settings)
	{
	}

	IndexBuffer::IndexBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings) : Buffer(globalContext, settings)
	{
	}
}
