#pragma once
#include <Core/Buffer.hpp>
#include <Core/Context.hpp>

#ifdef MMPENGINE_BACKEND_DX12
#include <Backend/Dx12/Buffer.hpp>
#endif

namespace MMPEngine::Frontend
{
	class UploadBuffer final : public Core::UploadBuffer
	{
	public:
		UploadBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		void Write(const void* src, std::size_t byteLength, std::size_t byteOffset = 0) override;
		std::shared_ptr<Buffer> GetUnderlyingBuffer() override;
	private:
		std::shared_ptr<Core::UploadBuffer> _impl;
	};


	class ReadBackBuffer final : public Core::ReadBackBuffer
	{
	public:
		ReadBackBuffer(const std::shared_ptr<Core::AppContext>& appContext, const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		void Read(void* dst, std::size_t byteLength, std::size_t byteOffset) override;
		std::shared_ptr<Buffer> GetUnderlyingBuffer() override;
	private:
		std::shared_ptr<Core::ReadBackBuffer> _impl;
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
}
