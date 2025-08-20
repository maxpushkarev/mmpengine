#pragma once
#include <Core/Buffer.hpp>
#include <Core/Context.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Buffer.hpp>
#endif

#ifdef MMPENGINE_BACKEND_VULKAN
#include <Backend/Vulkan/Buffer.hpp>
#endif

#ifdef MMPENGINE_BACKEND_METAL
#include <Backend/Metal/Buffer.hpp>
#endif


namespace MMPEngine::Frontend
{
	template<typename TCoreBuffer, typename TSettings = Core::Buffer::Settings>
	class Buffer : public TCoreBuffer
	{
	protected:
		Buffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const TSettings& settings);
		std::shared_ptr<TCoreBuffer> CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext);
	public:
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::Buffer> GetUnderlyingBuffer() override;
	protected:
		std::shared_ptr<TCoreBuffer> _impl;

	};

	class UploadBuffer : public Buffer<Core::UploadBuffer>
	{
	public:
		UploadBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings);
		std::shared_ptr<Core::ContextualTask<WriteTaskContext>> CreateWriteTask(const void* src, std::size_t byteLength, std::size_t byteOffset = 0) override;
	};


	class ReadBackBuffer : public Buffer<Core::ReadBackBuffer>
	{
	public:
		ReadBackBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings);
		std::shared_ptr<Core::ContextualTask<ReadTaskContext>> CreateReadTask(void* dst, std::size_t byteLength, std::size_t byteOffset = 0) override;
	};

	class ResidentBuffer : public Buffer<Core::ResidentBuffer>
	{
	public:
		ResidentBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings);
	};

	class UnorderedAccessBuffer : public Buffer<Core::UnorderedAccessBuffer, Core::BaseUnorderedAccessBuffer::Settings>
	{
	public:
		UnorderedAccessBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings);
	};

	class CounteredUnorderedAccessBuffer : public Buffer<Core::CounteredUnorderedAccessBuffer, Core::BaseUnorderedAccessBuffer::Settings>
	{
	public:
		CounteredUnorderedAccessBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateCopyCounterTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t dstByteOffset) override;
		std::shared_ptr<Core::BaseTask> CreateResetCounterTask() override;
	};;

	class VertexBuffer final : public Buffer<Core::VertexBuffer, Core::InputAssemblerBuffer::Settings>
	{
	public:
		VertexBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings);
	};

	class IndexBuffer final : public Buffer<Core::IndexBuffer, Core::InputAssemblerBuffer::Settings>
	{
	public:
		IndexBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Settings& settings);
	};

	class BaseStructuredBuffer
	{
	public:
		struct Settings final
		{
			std::size_t itemsCount = 0;
			std::string name = {};
		};
	};

	template<typename TStruct>
	class StructuredBuffer : public BaseStructuredBuffer
	{
		static_assert(std::is_pod_v<TStruct>, "TStruct must be POD");
		static_assert(std::is_final_v<TStruct>, "TStruct must be final");
	};


	template<typename TStruct>
	class StructuredUploadBuffer final : public StructuredBuffer<TStruct>, public UploadBuffer
	{
	public:
		StructuredUploadBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const BaseStructuredBuffer::Settings& settings);
		std::shared_ptr<Core::ContextualTask<Core::UploadBuffer::WriteTaskContext>> CreateWriteStructTask(const TStruct& item, std::size_t index);
        std::optional<std::size_t> GetStride() const override;
	};


	template<typename TStruct>
	class StructuredReadBackBuffer final : public StructuredBuffer<TStruct>, public ReadBackBuffer
	{
	public:
		StructuredReadBackBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const BaseStructuredBuffer::Settings& settings);
		std::shared_ptr<Core::ContextualTask<Core::ReadBackBuffer::ReadTaskContext>> CreateReadStructTask(TStruct& item, std::size_t index);
        std::optional<std::size_t> GetStride() const override;
	};

	template<typename TStruct>
	class StructuredResidentBuffer final : public StructuredBuffer<TStruct>, public ResidentBuffer
	{
	public:
		StructuredResidentBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const BaseStructuredBuffer::Settings& settings);
        std::optional<std::size_t> GetStride() const override;
	};


	template<class TUniformBufferData>
	class UniformBuffer final : public Core::UniformBuffer<TUniformBufferData>
	{
	public:
		UniformBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, std::string_view name);
		UniformBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext);

		std::shared_ptr<Core::ContextualTask<typename Core::UniformBuffer<TUniformBufferData>::WriteTaskContext>> CreateWriteAsyncTask(const TUniformBufferData& data) override;

		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::Buffer> GetUnderlyingBuffer() override;

	private:
		std::shared_ptr<Core::UniformBuffer<TUniformBufferData>> _impl;
	};

	template<class TUniformBufferData>
	inline UniformBuffer<TUniformBufferData>::UniformBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, std::string_view name) :
		Core::UniformBuffer<TUniformBufferData>(Core::Buffer::Settings {sizeof(typename Core::UniformBuffer<TUniformBufferData>::TData), std::string {name}})
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::UniformBuffer<TUniformBufferData>>(name);
#else
			throw Core::UnsupportedException("unable to create constant buffer for DX12 backend");
#endif
		}

		if (globalContext->settings.backend == Core::BackendType::Vulkan)
		{
#ifdef MMPENGINE_BACKEND_VULKAN
			_impl = std::make_shared<Backend::Vulkan::UniformBuffer<TUniformBufferData>>(name);
#else
			throw Core::UnsupportedException("unable to create constant buffer for Vulkan backend");
#endif
		}
            
        if (globalContext->settings.backend == Core::BackendType::Metal)
        {
#ifdef MMPENGINE_BACKEND_METAL
            _impl = std::make_shared<Backend::Metal::UniformBuffer<TUniformBufferData>>(name);
#else
            throw Core::UnsupportedException("unable to create constant buffer for Metal backend");
#endif
        }
	}

	template<class TUniformBufferData>
	inline UniformBuffer<TUniformBufferData>::UniformBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext) :
		Core::UniformBuffer<TUniformBufferData>(Core::Buffer::Settings {sizeof(typename Core::UniformBuffer<TUniformBufferData>::TData), ""})
	{
		if (globalContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::UniformBuffer<TUniformBufferData>>();
#else
			throw Core::UnsupportedException("unable to create constant buffer for DX12 backend");
#endif
		}

		if (globalContext->settings.backend == Core::BackendType::Vulkan)
		{
#ifdef MMPENGINE_BACKEND_VULKAN
			_impl = std::make_shared<Backend::Vulkan::UniformBuffer<TUniformBufferData>>();
#else
			throw Core::UnsupportedException("unable to create constant buffer for Vulkan backend");
#endif
		}
            
        if (globalContext->settings.backend == Core::BackendType::Metal)
        {
#ifdef MMPENGINE_BACKEND_METAL
            _impl = std::make_shared<Backend::Metal::UniformBuffer<TUniformBufferData>>();
#else
            throw Core::UnsupportedException("unable to create constant buffer for Metal backend");
#endif
        }
	}

	template <class TUniformBufferData>
	inline std::shared_ptr<Core::ContextualTask<typename Core::UniformBuffer<TUniformBufferData>::WriteTaskContext>> UniformBuffer<TUniformBufferData>::CreateWriteAsyncTask(const TUniformBufferData& data)
	{
		return _impl->CreateWriteAsyncTask(data);
	}

	template <class TUniformBufferData>
	inline std::shared_ptr<Core::BaseTask> UniformBuffer<TUniformBufferData>::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		return _impl->CreateCopyToBufferTask(dst, byteLength, srcByteOffset, dstByteOffset);
	}

	template <class TUniformBufferData>
	inline std::shared_ptr<Core::BaseTask> UniformBuffer<TUniformBufferData>::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();
	}

	template <class TUniformBufferData>
	inline std::shared_ptr<Core::Buffer> UniformBuffer<TUniformBufferData>::GetUnderlyingBuffer()
	{
		return _impl->GetUnderlyingBuffer();
	}

	template<>
	std::shared_ptr<Core::UploadBuffer> Buffer<Core::UploadBuffer>::CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext);
	template<>
	std::shared_ptr<Core::ResidentBuffer> Buffer<Core::ResidentBuffer>::CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext);
	template<>
	std::shared_ptr<Core::ReadBackBuffer> Buffer<Core::ReadBackBuffer>::CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext);
	template<>
	std::shared_ptr<Core::UnorderedAccessBuffer> Buffer<Core::UnorderedAccessBuffer, Core::BaseUnorderedAccessBuffer::Settings>::CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext);
	template<>
	std::shared_ptr<Core::CounteredUnorderedAccessBuffer> Buffer<Core::CounteredUnorderedAccessBuffer, Core::BaseUnorderedAccessBuffer::Settings>::CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext);
	template<>
	std::shared_ptr<Core::VertexBuffer> Buffer<Core::VertexBuffer, Core::InputAssemblerBuffer::Settings>::CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext);
	template<>
	std::shared_ptr<Core::IndexBuffer> Buffer<Core::IndexBuffer, Core::InputAssemblerBuffer::Settings>::CreateImpl(const std::shared_ptr<Core::GlobalContext>& globalContext);

	template<>
	Buffer<Core::VertexBuffer, Core::InputAssemblerBuffer::Settings>::Buffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Core::InputAssemblerBuffer::Settings& settings);
	template<>
	Buffer<Core::IndexBuffer, Core::InputAssemblerBuffer::Settings>::Buffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const Core::InputAssemblerBuffer::Settings& settings);

	template <typename TCoreBuffer, typename TSettings>
	Buffer<TCoreBuffer, TSettings>::Buffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const TSettings& settings)
		: TCoreBuffer(settings)
	{
		_impl = CreateImpl(globalContext);
	}

	template <typename TCoreBuffer, typename TSettings>
	std::shared_ptr<Core::BaseTask> Buffer<TCoreBuffer, TSettings>::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();
	}

	template <typename TCoreBuffer, typename TSettings>
	std::shared_ptr<Core::Buffer> Buffer<TCoreBuffer, TSettings>::GetUnderlyingBuffer()
	{
		return _impl->GetUnderlyingBuffer();
	}

	template <typename TCoreBuffer, typename TSettings>
	std::shared_ptr<Core::BaseTask> Buffer<TCoreBuffer, TSettings>::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		return _impl->CreateCopyToBufferTask(dst, byteLength, srcByteOffset, dstByteOffset);
	}

	template <typename TStruct>
	StructuredUploadBuffer<TStruct>::StructuredUploadBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const BaseStructuredBuffer::Settings& settings)
		: UploadBuffer(globalContext, Core::Buffer::Settings{sizeof(TStruct)* settings.itemsCount, settings.name})
	{
	}

    template <typename TStruct>
    std::optional<std::size_t> StructuredUploadBuffer<TStruct>::GetStride() const
    {
        return sizeof(TStruct);
    }

	template <typename TStruct>
	std::shared_ptr<Core::ContextualTask<Core::UploadBuffer::WriteTaskContext>> StructuredUploadBuffer<TStruct>::CreateWriteStructTask(const TStruct& item, std::size_t index)
	{
		return this->CreateWriteTask(std::addressof(item), sizeof(TStruct), sizeof(TStruct) * index);
	}

	template <typename TStruct>
	StructuredReadBackBuffer<TStruct>::StructuredReadBackBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const BaseStructuredBuffer::Settings& settings)
		: ReadBackBuffer(globalContext, Core::Buffer::Settings{sizeof(TStruct)* settings.itemsCount, settings.name})
	{
	}

    template <typename TStruct>
    std::optional<std::size_t> StructuredReadBackBuffer<TStruct>::GetStride() const
    {
        return sizeof(TStruct);
    }

	template <typename TStruct>
	std::shared_ptr<Core::ContextualTask<Core::ReadBackBuffer::ReadTaskContext>> StructuredReadBackBuffer<TStruct>::CreateReadStructTask(TStruct& item, std::size_t index)
	{
		return this->CreateReadTask(std::addressof(item), sizeof(TStruct), sizeof(TStruct) * index);
	}

	template <typename TStruct>
	StructuredResidentBuffer<TStruct>::StructuredResidentBuffer(const std::shared_ptr<Core::GlobalContext>& globalContext, const BaseStructuredBuffer::Settings& settings)
		: ResidentBuffer(globalContext, Core::Buffer::Settings{sizeof(TStruct)* settings.itemsCount, settings.name})
	{
	}

    template <typename TStruct>
    std::optional<std::size_t> StructuredResidentBuffer<TStruct>::GetStride() const
    {
        return sizeof(TStruct);
    }
}
