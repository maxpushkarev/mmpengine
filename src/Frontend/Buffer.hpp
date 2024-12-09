#pragma once
#include <Core/Buffer.hpp>
#include <Core/Context.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Buffer.hpp>
#endif

namespace MMPEngine::Frontend
{
	template<typename TCoreBuffer, typename TSettings = Core::Buffer::Settings>
	class Buffer : public TCoreBuffer
	{
	protected:
		Buffer(const std::shared_ptr<Core::AppContext>& appContext, const TSettings& settings);
		std::shared_ptr<TCoreBuffer> CreateImpl(const std::shared_ptr<Core::AppContext>& appContext);
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
		UploadBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings);
		void Write(const void* src, std::size_t byteLength, std::size_t byteOffset = 0) override;
	};


	class ReadBackBuffer : public Buffer<Core::ReadBackBuffer>
	{
	public:
		ReadBackBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings);
		void Read(void* dst, std::size_t byteLength, std::size_t byteOffset) override;
	};

	class ResidentBuffer : public Buffer<Core::ResidentBuffer>
	{
	public:
		ResidentBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings);
	};

	class VertexBuffer final : public Buffer<Core::VertexBuffer, Core::InputAssemblerBuffer::Settings>
	{
	public:
		VertexBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings);
	};

	class IndexBuffer final : public Buffer<Core::IndexBuffer, Core::InputAssemblerBuffer::Settings>
	{
	public:
		IndexBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings);
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
		StructuredUploadBuffer(const std::shared_ptr<Core::AppContext>& appContext, const BaseStructuredBuffer::Settings& settings);
		void WriteStruct(const TStruct& item, std::size_t index);
	};


	template<typename TStruct>
	class StructuredReadBackBuffer final : public StructuredBuffer<TStruct>, public ReadBackBuffer
	{
	public:
		StructuredReadBackBuffer(const std::shared_ptr<Core::AppContext>& appContext, const BaseStructuredBuffer::Settings& settings);
		void ReadStruct(TStruct& item, std::size_t index);
	};

	template<typename TStruct>
	class StructuredResidentBuffer final : public StructuredBuffer<TStruct>, public ResidentBuffer
	{
	public:
		StructuredResidentBuffer(const std::shared_ptr<Core::AppContext>& appContext, const BaseStructuredBuffer::Settings& settings);
	};


	template<class TConstantBufferData>
	class ConstantBuffer final : public Core::ConstantBuffer<TConstantBufferData>
	{
	public:
		ConstantBuffer(const std::shared_ptr<Core::AppContext>& appContext, std::string_view name);
		ConstantBuffer(const std::shared_ptr<Core::AppContext>& appContext);
		void Write(const TConstantBufferData& data) override;

		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::Buffer> GetUnderlyingBuffer() override;

	private:
		std::shared_ptr<Core::ConstantBuffer<TConstantBufferData>> _impl;
	};

	template<class TConstantBufferData>
	inline ConstantBuffer<TConstantBufferData>::ConstantBuffer(const std::shared_ptr<Core::AppContext>& appContext, std::string_view name) : Core::BaseEntity(name),
		Core::ConstantBuffer<TConstantBufferData>(Core::Buffer::Settings {sizeof(TConstantBufferData), std::string {name}})
	{
		if (appContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::ConstantBuffer<TConstantBufferData>>(name);
#else
			throw Core::UnsupportedException("unable to create constant buffer for DX12 backend");
#endif
		}
	}

	template<class TConstantBufferData>
	inline ConstantBuffer<TConstantBufferData>::ConstantBuffer(const std::shared_ptr<Core::AppContext>& appContext) : Core::BaseEntity(),
		Core::ConstantBuffer<TConstantBufferData>(Core::Buffer::Settings {sizeof(TConstantBufferData), ""})
	{
		if (appContext->settings.backend == Core::BackendType::Dx12)
		{
#ifdef MMPENGINE_BACKEND_DX12
			_impl = std::make_shared<Backend::Dx12::ConstantBuffer<TConstantBufferData>>();
#else
			throw Core::UnsupportedException("unable to create constant buffer for DX12 backend");
#endif
		}
	}

	template <class TConstantBufferData>
	void ConstantBuffer<TConstantBufferData>::Write(const TConstantBufferData& data)
	{
		_impl->Write(data);
	}

	template <class TConstantBufferData>
	std::shared_ptr<Core::BaseTask> ConstantBuffer<TConstantBufferData>::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		return _impl->CreateCopyToBufferTask(dst, byteLength, srcByteOffset, dstByteOffset);
	}

	template <class TConstantBufferData>
	std::shared_ptr<Core::BaseTask> ConstantBuffer<TConstantBufferData>::CreateInitializationTask()
	{
		return _impl->CreateInitializationTask();
	}

	template <class TConstantBufferData>
	std::shared_ptr<Core::Buffer> ConstantBuffer<TConstantBufferData>::GetUnderlyingBuffer()
	{
		return _impl->GetUnderlyingBuffer();
	}

	template<>
	std::shared_ptr<Core::UploadBuffer> Buffer<Core::UploadBuffer>::CreateImpl(const std::shared_ptr<Core::AppContext>& appContext);
	template<>
	std::shared_ptr<Core::ResidentBuffer> Buffer<Core::ResidentBuffer>::CreateImpl(const std::shared_ptr<Core::AppContext>& appContext);
	template<>
	std::shared_ptr<Core::ReadBackBuffer> Buffer<Core::ReadBackBuffer>::CreateImpl(const std::shared_ptr<Core::AppContext>& appContext);
	template<>
	std::shared_ptr<Core::VertexBuffer> Buffer<Core::VertexBuffer, Core::InputAssemblerBuffer::Settings>::CreateImpl(const std::shared_ptr<Core::AppContext>& appContext);
	template<>
	std::shared_ptr<Core::IndexBuffer> Buffer<Core::IndexBuffer, Core::InputAssemblerBuffer::Settings>::CreateImpl(const std::shared_ptr<Core::AppContext>& appContext);

	template<>
	Buffer<Core::VertexBuffer, Core::InputAssemblerBuffer::Settings>::Buffer(const std::shared_ptr<Core::AppContext>& appContext, const Core::InputAssemblerBuffer::Settings& settings);
	template<>
	Buffer<Core::IndexBuffer, Core::InputAssemblerBuffer::Settings>::Buffer(const std::shared_ptr<Core::AppContext>& appContext, const Core::InputAssemblerBuffer::Settings& settings);

	template <typename TCoreBuffer, typename TSettings>
	Buffer<TCoreBuffer, TSettings>::Buffer(const std::shared_ptr<Core::AppContext>& appContext, const TSettings& settings)
		: TCoreBuffer(settings)
	{
		_impl = CreateImpl(appContext);
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
	StructuredUploadBuffer<TStruct>::StructuredUploadBuffer(const std::shared_ptr<Core::AppContext>& appContext, const BaseStructuredBuffer::Settings& settings)
		: UploadBuffer(appContext, Core::Buffer::Settings{sizeof(TStruct)* settings.itemsCount, settings.name})
	{
	}

	template <typename TStruct>
	void StructuredUploadBuffer<TStruct>::WriteStruct(const TStruct& item, std::size_t index)
	{
		this->Write(std::addressof(item), sizeof(TStruct), sizeof(TStruct) * index);
	}

	template <typename TStruct>
	StructuredReadBackBuffer<TStruct>::StructuredReadBackBuffer(const std::shared_ptr<Core::AppContext>& appContext, const BaseStructuredBuffer::Settings& settings)
		: ReadBackBuffer(appContext, Core::Buffer::Settings{sizeof(TStruct)* settings.itemsCount, settings.name})
	{
	}

	template <typename TStruct>
	void StructuredReadBackBuffer<TStruct>::ReadStruct(TStruct& item, std::size_t index)
	{
		this->Read(std::addressof(item), sizeof(TStruct), sizeof(TStruct) * index);
	}

	template <typename TStruct>
	StructuredResidentBuffer<TStruct>::StructuredResidentBuffer(const std::shared_ptr<Core::AppContext>& appContext, const BaseStructuredBuffer::Settings& settings)
		: ResidentBuffer(appContext, Core::Buffer::Settings{sizeof(TStruct)* settings.itemsCount, settings.name})
	{
	}
}
