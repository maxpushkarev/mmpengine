#pragma once
#include <Core/Buffer.hpp>
#include <Backend/Dx12/Entity.hpp>
#include "Buffer.hpp"

namespace MMPEngine::Backend::Dx12
{
	class CopyBufferTaskContext : public Core::TaskContext
	{
	public:
		std::weak_ptr<Dx12::BaseEntity> src;
		std::weak_ptr<Dx12::BaseEntity> dst;
		std::size_t byteLength = 0;
		std::size_t srcByteOffset = 0;
		std::size_t dstByteOffset = 0;
	};

	class CopyBufferTask final : public Task, public Core::TaskWithInternalContext<CopyBufferTaskContext>
	{
	public:
		CopyBufferTask(const std::shared_ptr<CopyBufferTaskContext>& context);
		void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		void Finalize(const std::shared_ptr<Core::BaseStream>& stream) override;
	private:
		std::shared_ptr<BaseTask> _switchSrcStateTask;
		std::shared_ptr<BaseTask> _switchDstStateTask;
	};

	class MappedBuffer : public ResourceEntity
	{
	public:
		MappedBuffer(std::string_view name);
		MappedBuffer();
		MappedBuffer(const MappedBuffer&) = delete;
		MappedBuffer(MappedBuffer&&) noexcept = delete;
		MappedBuffer& operator=(const MappedBuffer&) = delete;
		MappedBuffer& operator=(MappedBuffer&&) noexcept = delete;
		~MappedBuffer() override;
	protected:

		class InitTaskContext final : public InitContext<MappedBuffer>
		{
		public:
			D3D12_HEAP_TYPE heapType;
			std::size_t byteSize;
		};

		class InitTask final : public Task, public Core::TaskWithInternalContext<InitTaskContext>
		{
		public:
			InitTask(const std::shared_ptr<InitTaskContext>& context);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			void Finalize(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		void Map();
		void Unmap();
		void* _mappedBufferPtr = nullptr;
	};

	class UploadBuffer final : public Core::UploadBuffer, public MappedBuffer
	{
	public:
		UploadBuffer(const Settings& settings);
		void Write(const void* src, std::size_t byteLength, std::size_t byteOffset) override;
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	};

	class ReadBackBuffer final : public Core::ReadBackBuffer, public MappedBuffer
	{
	public:
		ReadBackBuffer(const Settings& settings);
		void Read(void* dst, std::size_t byteLength, std::size_t byteOffset) override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
	};


	template<class TConstantBufferData>
	class ConstantBuffer final : public Core::ConstantBuffer<TConstantBufferData>
	{
	public:
		ConstantBuffer(std::string_view name);
		ConstantBuffer();
		void Write(const TConstantBufferData& data) override;

		std::shared_ptr<Core::BaseTask> CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
		std::shared_ptr<Core::Buffer> GetUnderlyingBuffer() override;

	private:
		static std::size_t GetRequiredSize();
		std::shared_ptr<UploadBuffer> _uploadBuffer;
	};

	template<class TConstantBufferData>
	inline ConstantBuffer<TConstantBufferData>::ConstantBuffer(std::string_view name) : Core::BaseEntity(name),
		Core::ConstantBuffer<TConstantBufferData>(Core::Buffer::Settings {GetRequiredSize(), std::string {name}})
	{
		_uploadBuffer = std::make_shared<UploadBuffer>(this->_settings);
	}

	template<class TConstantBufferData>
	inline ConstantBuffer<TConstantBufferData>::ConstantBuffer() : Core::BaseEntity(""),
		Core::ConstantBuffer<TConstantBufferData>(Core::Buffer::Settings {GetRequiredSize(), ""})
	{
		_uploadBuffer = std::make_shared<UploadBuffer>(this->_settings);
	}

	template<class TConstantBufferData>
	inline void ConstantBuffer<TConstantBufferData>::Write(const TConstantBufferData& data)
	{
		_uploadBuffer->Write(std::addressof(data), sizeof(data), 0);
	}

	template<class TConstantBufferData>
	inline std::shared_ptr<Core::BaseTask> ConstantBuffer<TConstantBufferData>::CreateCopyToBufferTask(const std::shared_ptr<Core::Buffer>& dst, std::size_t byteLength, std::size_t srcByteOffset, std::size_t dstByteOffset) const
	{
		return _uploadBuffer->CreateCopyToBufferTask(dst->GetUnderlyingBuffer(), byteLength, srcByteOffset, dstByteOffset);
	}

	template<class TConstantBufferData>
	inline std::shared_ptr<Core::BaseTask> ConstantBuffer<TConstantBufferData>::CreateInitializationTask()
	{
		return _uploadBuffer->CreateInitializationTask();
	}

	template<class TConstantBufferData>
	inline std::shared_ptr<Core::Buffer> ConstantBuffer<TConstantBufferData>::GetUnderlyingBuffer()
	{
		return _uploadBuffer;
	}

	template<class TConstantBufferData>
	inline std::size_t ConstantBuffer<TConstantBufferData>::GetRequiredSize()
	{
		// Constant buffers must be a multiple of the minimum hardware
		// allocation size (usually 256 bytes).  So round up to nearest
		// multiple of 256.  We do this by adding 255 and then masking off
		// the lower 2 bytes which store all bits < 256.
		// Example: Suppose byteSize = 300.
		// (300 + 255) & ~255
		// 555 & ~255
		// 0x022B & ~0x00ff
		// 0x022B & 0xff00
		// 0x0200
		// 512
		return (sizeof(TConstantBufferData) + 255) & ~255;
	}

}