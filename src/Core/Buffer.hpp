#pragma once
#include <Core/Entity.hpp>

namespace MMPEngine::Core
{
	class Buffer : public virtual BaseEntity
	{
	public:
		struct Settings final
		{
			std::size_t byteLength;
			std::string name = {};
		};
		virtual std::shared_ptr<BaseTask> CreateCopyToBufferTask(
			const std::shared_ptr<Buffer>& dst, 
			std::size_t byteLength, 
			std::size_t srcByteOffset, 
			std::size_t dstByteOffset
		) const = 0;
		std::shared_ptr<BaseTask> CopyToBuffer(const std::shared_ptr<Buffer>& dst) const;
		std::shared_ptr<Buffer> GetUnderlyingBuffer() const;
		virtual std::shared_ptr<Buffer> GetUnderlyingBuffer();
		const Settings& GetSettings() const;
	protected:
		const Settings& GetSettingsInternal();
		Buffer(const Settings& settings);
		Settings _settings;
	};

	class UploadBuffer : public Buffer
	{
	protected:
		UploadBuffer(const Settings& settings);
	public:
		virtual void Write(const void* src, std::size_t byteLength, std::size_t byteOffset = 0) = 0;
	};

	class ReadBackBuffer : public Buffer
	{
	protected:
		ReadBackBuffer(const Settings& settings);
	public:
		virtual void Read(void* dst, std::size_t byteLength, std::size_t byteOffset = 0) = 0;
	};

	class ResidentBuffer : public Buffer
	{
	protected:
		ResidentBuffer(const Settings& settings);
	};

	class InputAssemblerBuffer : public Buffer
	{
	public:
		struct IASettings final
		{
			const void* rawData;
		};
		struct Settings final
		{
			IASettings ia;
			Buffer::Settings base;
		};
	protected:
		InputAssemblerBuffer(const Settings& settings);
		IASettings _ia;
	};

	class VertexBuffer : public virtual InputAssemblerBuffer
	{
	protected:
		VertexBuffer(const Settings& settings);
	};

	class IndexBuffer : public virtual InputAssemblerBuffer
	{
	protected:
		IndexBuffer(const Settings& settings);
	};

	template<class TConstantBufferData>
	class ConstantBuffer: public Buffer
	{
	protected:
		using TData = std::decay_t<TConstantBufferData>;
		static_assert(std::is_final_v<TData>, "TData should be final");
		static_assert(std::is_pod_v<TData>, "TData should be POD");

		ConstantBuffer(const Settings& settings);

	public:
		void virtual Write(const TData& data) = 0;
	};

	template<class TConstantBufferData>
	inline ConstantBuffer<TConstantBufferData>::ConstantBuffer(const Settings& settings) : Buffer(settings)
	{
	}
}